# TCP Client-Server app

Requirements:
- Docker

Container tech stack:
- Ubuntu 22.04
- CMake
- C++23

Run server:

```
./console_server ${port}
```

Run client:

```
./console_client ${client title} ${port} ${reconnecting to server timeout in seconds}
```