#pragma once

constexpr size_t RESPONSE_BUFFER_LENGTH = 512;
constexpr size_t REQUEST_BUFFER_LENGTH = 512;

constexpr size_t INITIAL_HEADER_LENGTH = 2048;//2KB

constexpr size_t INCREMENT_MESSAGE_SIZE = 4;

constexpr unsigned short PORT = 3000; 

constexpr unsigned short WORKER_THREADS_COUNT = 15;
constexpr unsigned short IOTHREADS_COUNT = 15;
