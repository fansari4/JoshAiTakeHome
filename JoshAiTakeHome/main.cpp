#include "pch.h"

using json = nlohmann::json;

namespace {

struct Light {
    std::string id;
    std::string name;
    std::string room;
    bool on = false;
    int brightness = 0;
};

std::atomic<bool> g_running{true};

void PrintJsonObject(const json& obj) {
    std::cout << obj.dump() << '\n';
}

void PrintFullLight(const Light& light) {
    json data = {
        {"name", light.name},
        {"id", light.id},
        {"room", light.room},
        {"on", light.on},
        {"brightness", light.brightness}
    };
    PrintJsonObject(data);
}

void PrintPropertyChange(const std::string& id, const std::string& key, const json& value) {
    json change = { {"id", id}, {key, value} };
    PrintJsonObject(change);
}

int ConvertBrightnessToPercent(std::uint8_t apiValue) {
    return static_cast<int>(std::round(apiValue * 100.0 / 255.0));
}

void SignalHandler(int) {
    g_running = false;
}

std::vector<Light> ParseSummaries(const json& payload) {
    std::vector<Light> lights;
    if (!payload.is_array()) {
        return lights;
    }

    for (const auto& item : payload) {
        if (!item.is_object()) {
            continue;
        }

        Light light;
        light.id = item.value("id", "");
        light.name = item.value("name", "");
        light.room = item.value("room", "");
        lights.push_back(light);
    }
    return lights;
}

bool TryGetLightById(const std::string& id, Light& outLight, const std::string& host, const int port) {
    const std::string url = "/lights/" + id;
    httplib::Client client(host, port);
    client.set_follow_location(true);
    client.set_read_timeout(5);
    client.set_write_timeout(5);

    auto res = client.Get(url.c_str());
    if (!res) {
        std::cerr << "ERROR: request failed, retrying...\n";
        return false;
    }
    if (res->status == 404) {
        return false;
    }
    if (res->status != 200) {
        std::cerr << "ERROR: request failed, retrying...\n";
        return false;
    }

    try {
        const auto payload = json::parse(res->body);
        if (!payload.is_object()) {
            std::cerr << "ERROR: invalid JSON response\n";
            return false;
        }

        outLight.id = payload.value("id", "");
        outLight.name = payload.value("name", "");
        outLight.room = payload.value("room", "");
        outLight.on = payload.value("on", false);

        if (payload.contains("brightness") && payload["brightness"].is_number_integer()) {
            const int apiBrightness = payload["brightness"].get<int>();
            outLight.brightness = ConvertBrightnessToPercent(static_cast<std::uint8_t>(apiBrightness));
        }
        return true;
    } catch (const std::exception&) {
        std::cerr << "ERROR: invalid JSON response\n";
        return false;
    }
}

bool TryGetAllLights(std::vector<Light>& lights, const std::string& host, const int port) {
    httplib::Client client(host, port);
    client.set_follow_location(true);
    client.set_read_timeout(5);
    client.set_write_timeout(5);

    auto res = client.Get("/lights");
    if (!res) {
        std::cerr << "ERROR: request failed, retrying...\n";
        return false;
    }
    if (res->status != 200) {
        std::cerr << "ERROR: request failed, retrying...\n";
        return false;
    }

    try {
        const auto payload = json::parse(res->body);
        const auto summaries = ParseSummaries(payload);
        lights = summaries;

        for (auto& light : lights) {
            Light full;
            if (TryGetLightById(light.id, full, host, port)) {
                light = full;
            }
        }
        return true;
    } catch (const std::exception&) {
        std::cerr << "ERROR: invalid JSON response\n";
        return false;
    }
}

std::unordered_map<std::string, Light> PollLights(const std::string& host, const int port) {
    std::vector<Light> fetched;
    if (!TryGetAllLights(fetched, host, port)) {
        return {};
    }

    std::unordered_map<std::string, Light> result;
    for (const auto& light : fetched) {
        if (!light.id.empty()) {
            result[light.id] = light;
        }
    }
    return result;
}

void EmitAddedLight(const Light& light) {
    PrintFullLight(light);
}

void EmitRemovedLight(const Light& light) {
    std::cout << light.name << " (" << light.id << ") has been removed\n";
}

void EmitDiffs(const std::unordered_map<std::string, Light>& oldLights,
               const std::unordered_map<std::string, Light>& newLights) {
    std::unordered_set<std::string> newIds;

    for (const auto& [id, light] : newLights) {
        newIds.insert(id);

        const auto it = oldLights.find(id);
        if (it == oldLights.end()) {
            EmitAddedLight(light);
            continue;
        }

        const Light& oldLight = it->second;
        if (oldLight.name != light.name) {
            PrintPropertyChange(id, "name", light.name);
        }
        if (oldLight.room != light.room) {
            PrintPropertyChange(id, "room", light.room);
        }
        if (oldLight.on != light.on) {
            PrintPropertyChange(id, "on", light.on);
        }
        if (oldLight.brightness != light.brightness) {
            PrintPropertyChange(id, "brightness", light.brightness);
        }
    }

    for (const auto& [id, light] : oldLights) {
        if (!newIds.count(id)) {
            EmitRemovedLight(light);
        }
    }
}

void PrintInitialState(const std::unordered_map<std::string, Light>& lights) {
    std::cout << "[\n";
    bool first = true;
    for (const auto& [id, light] : lights) {
        if (!first) {
            std::cout << ",\n";
        }
        first = false;

        json item = {
            {"name", light.name},
            {"id", light.id},
            {"room", light.room},
            {"on", light.on},
            {"brightness", light.brightness}
        };
        std::cout << item.dump(4);
    }
    std::cout << "\n]\n";
}

} // namespace

int main(int argc, char** argv) {
    std::string host = "localhost";
    int port = 8080;
    int intervalMs = 1000;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--interval" && i + 1 < argc) {
            intervalMs = std::stoi(argv[++i]);
        }
    }

    std::signal(SIGINT, SignalHandler);

    auto current = PollLights(host, port);
    if (!current.empty()) {
        PrintInitialState(current);
    }

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));

        auto latest = PollLights(host, port);
        if (latest.empty() && !current.empty()) {
            std::cerr << "ERROR: request failed, retrying...\n";
            continue;
        }

        if (!latest.empty()) {
            EmitDiffs(current, latest);
            current = std::move(latest);
        }
    }

    std::cout << "Stopping monitor...\n";
    return 0;
}
