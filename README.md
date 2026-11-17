# Josh.ai Light Monitor

This project is a C++17 console application that polls the Josh.ai lights simulator, prints the initial light state, and emits JSON change notifications for added, removed, and updated lights.

## Features

- Polls `GET /lights` and `GET /lights/{id}`
- Converts API brightness from 0-255 to 0-100%
- Detects:
  - light additions
  - light removals
  - `on`/`off` changes
  - brightness changes
  - name/room changes
- Gracefully handles HTTP and JSON failures without exiting
- Supports `--host`, `--port`, and `--interval` options

## Build

### Linux/macOS

```bash
make
```

### Windows (MSVC)

From a Developer PowerShell / Developer Command Prompt:

```powershell
cl /nologo /EHsc /std:c++17 /I. /Iinclude main.cpp /Fe:light_monitor.exe
```

## Run

Start the simulator first, then run:

```bash
./light_monitor --host localhost --port 8080 --interval 1000
```

Or on Windows:

```powershell
.\light_monitor.exe --host localhost --port 8080 --interval 1000
```

## Binary

A ready-to-run Windows binary is included in this project directory:

```text
light_monitor.exe
```

## Notes

- The application keeps running until Ctrl+C is pressed.
- If the simulator is temporarily unavailable, it logs an error and retries on the next polling cycle.
- The monitor uses a single `unordered_map` snapshot keyed by light ID for efficient diffing.
