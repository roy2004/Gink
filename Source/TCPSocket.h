#pragma once

#include <climits>
#include <cstdint>
#include <cstddef>
#include <cassert>


class Stream;


class TCPSocket final
{
    explicit TCPSocket(const TCPSocket &) = delete;
    void operator=(const TCPSocket &) = delete;

public:
    inline explicit TCPSocket(int);

    explicit TCPSocket();
    ~TCPSocket();

    void listen(const char *, const char *, int = INT_MAX);
    int accept(std::uint32_t *, int *, int = -1);
    void connect(const char *, const char *, int = -1);
    std::size_t read(Stream *, std::size_t = 65536, int = -1);
    std::size_t write(Stream *, int = -1);

private:
    const int fd_;
};


TCPSocket::TCPSocket(int fd)
    : fd_(fd)
{
    assert(fd >= 0);
}
