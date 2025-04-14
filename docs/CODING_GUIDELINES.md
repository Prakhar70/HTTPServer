# Coding Guidelines for HTTPServer

This project aims to be a high-performance, production-grade HTTP server on Windows. Consistency, clarity, and system-level safety are key priorities.

---

## 🔤 Naming Conventions

| Type         | Convention       | Example                        |
|--------------|------------------|--------------------------------|
| Class        | PascalCase       | `HttpServer`, `AsyncHandler`   |
| Function     | camelCase        | `initializeServer()`, `runLoop()` |
| Local Var    | camelCase        | `bufferSize`, `isReady`        |
| Member Var   | `m_` prefix      | `m_threadPool`, `m_context`    |
| Global Var   | `g_` prefix      | `g_serverInstance`             |
| Constant     | UPPER_CASE       | `DEFAULT_PORT`, `MAX_THREADS`  |
| Pointer Var  | Optional `Ptr`   | `contextPtr`, `sockPtr`        |
| Enum         | PascalCase Type, UPPER_CASE values | `enum class State { IDLE, RUNNING }` |
| Headers      | snake_case       | `socket_manager.hpp`, `sync_flag.hpp` |

---

## 📦 File Layout

- `main.cpp` – Minimal entry point
- `service.cpp` – Windows Service + console fallback logic
- `llmserver.cpp` – IOCP HTTP server core
- `tasynchndlr.cpp` – Async queue dispatcher
- `tthreader.cpp` – Thread pool for worker threads
- `syncflag.cpp` – Custom reader-writer locking
- `test_*.cpp` – Unit tests

---

## 🧠 Best Practices

- Use `TCHAR`, `LPTSTR`, `TEXT()` for Unicode-aware code
- Avoid hardcoding `L""` or `"..."` — use `TEXT()` macros
- Keep functions small and logically scoped
- Use smart comments: explain *why*, not just *what*
- When casting, prefer `static_cast<>` over C-style
- No global new/delete unless wrapped in RAII or smart pointers

---

## ✅ Example Style

```cpp
void HttpServer::initialize(uint16_t port) {
    if (isRunning) return;

    m_port = port;
    setupIOCP();
    spawnWorkerThreads(DEFAULT_THREAD_COUNT);
}
