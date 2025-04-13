# High-Performance HTTP Server (Windows - C++)

This project implements a lightweight, high-performance HTTP server written from scratch in modern C++. It's designed specifically for Windows platforms using raw Win32 APIs and IOCP (I/O Completion Ports) for efficient asynchronous request handling. The server aims to provide an extremely scalable, low-latency web-serving solution with minimal dependencies.

## 🚀 Features

- **Non-blocking IO (IOCP)**: Efficiently handles thousands of simultaneous connections.
- **Custom Thread Pool**: Event-driven thread management ensuring resource efficiency.
- **Custom Assembly Spinlock**: Lightweight and high-performance synchronization for concurrent access, initially implemented in 32-bit assembly with plans for a 64-bit version.
- **HTTP/1.1 Compliant**: Supports standard HTTP requests and responses.
- **Modular Architecture**: Highly maintainable and easily extendable design.

## ⚙️ Technical Stack

- **Language**: C++17
- **Platform**: Windows (x86)
- **Concurrency**: IOCP, custom spinlocks, and thread pooling
- **Libraries**: Pure Win32 API (no external libraries)

## 📌 Quick Start

### Requirements

- Windows OS
- Visual Studio 2022 (recommended) or any modern C++ compiler supporting MSVC build tools
- CMake (3.15+)

### Building

```bash
# Clone the repository
git clone https://github.com/YourUsername/HTTPServer.git
cd HTTPServer

# Generate and build project
cmake -S . -B build -A Win32
cmake --build build --config Release
```

### Running the Server

Run the executable from the build output:

```bash
./build/Release/HTTPServer.exe
```

The server listens by default on port **8080**.

### Testing with curl

To test the server quickly:

```bash
curl -v http://localhost:8080/
```

You should see a simple HTTP response:

```
HTTP/1.1 200 OK
Content-Length: 13
Content-Type: text/plain

Hello, world!
```

## 🛠️ Project Highlights

### IOCP-Based Concurrency

Utilizes Windows IOCP for asynchronous I/O operations, enabling highly scalable handling of simultaneous network connections without traditional multi-threading overhead.

### Custom Assembly Locking Mechanism

Employs a hand-crafted spinlock written in 32-bit assembly, designed for extremely efficient read/write synchronization. A 64-bit version is planned for future releases.

### Modular and Extensible

Clean architecture facilitates the addition of new features such as routing, TLS encryption, gzip compression, or JSON handling.

## 🚧 Future Roadmap

- 64-bit implementation of spinlock synchronization
- Advanced HTTP routing capabilities
- Static file serving
- TLS/SSL support
- HTTP/2 support
- Enhanced logging and monitoring features

## 🤝 Contribution

Initially, this project is being privately developed by me. Soon, we intend to make it open-source and invite contributions from the broader C++ community.

If you're interested in contributing early or have feedback, please contact:

- **Prakhar Agarwal** (prakharagarwal70@gmail.com)

## 📜 License

Currently proprietary—open-source licensing is planned in future iterations.

---

Happy coding! 🚀