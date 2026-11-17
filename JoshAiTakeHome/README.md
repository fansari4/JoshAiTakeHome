# Josh.ai Light Monitor

This project is a C++14 console application that polls the Josh.ai lights simulator, prints the initial light state, and emits JSON change notifications for added, removed, and updated lights.

## Features

- Polls `GET /lights` and `GET /lights/{id}`
- Converts simulator brightness from `0-255` to the required `0-100` percentage
- Detects:
  - light additions
  - light removals
  - `on`/`off` changes
  - brightness changes
  - name/room changes
- Gracefully handles HTTP and JSON errors without crashing
- Supports `--host`, `--port`, and `--interval` command-line options

## Prerequisites

1. Start the Lights Simulator application.
2. Confirm it is running at:

```text
http://localhost:8080
```

## Run on Windows

From Command Prompt as administrator, go to the project directory:

```
cd C:\Users\User\source\repos\JoshAiTakeHome\JoshAiTakeHome
```

Then run the ready-made executable:

```
.\light_monitor.exe
```

This will use the default values:

- host: `localhost`
- port: `8080`
- interval: `1000` ms

You can also pass the values explicitly:

```
.\light_monitor.exe --host localhost --port 8080 --interval 1000
```

The monitor will print the initial light list and continue polling for changes until you press `Ctrl+C`.


## Binary

The project includes a ready-to-run Windows binary in the same folder as the source:

```text
C:\Users\User\source\repos\JoshAiTakeHome\JoshAiTakeHome\light_monitor.exe
```
