# TCP Client-Server app

Requirements:
- Docker

Container tech stack:
- Ubuntu 22.04
- CMake
- C++23

Memory leaks checked by Valgrind

Build:
```
mkdir build
cd build
cmake ..
cmake --build .
```

Run server:

```
cd build/apps/server/console_server

./console_server ${port}
```

Run client:

```
cd build/apps/client/console_client

./console_client ${client title} ${port} ${reconnecting to server timeout in seconds}
```