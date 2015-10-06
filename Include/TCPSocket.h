#pragma once


#include <climits>
#include <cstddef>

#include "IPEndpoint.h"


namespace Gink {

class Stream;


class TCPSocket final
{
    TCPSocket(const TCPSocket &) = delete;
    void operator=(const TCPSocket &) = delete;

public:
    inline TCPSocket(TCPSocket &&);

    static TCPSocket Listen(const char *, const char *, int = INT_MAX);
    static TCPSocket Connect(const char *, const char *, int = -1);

    ~TCPSocket();

    TCPSocket accept(IPEndpoint * = nullptr, int = -1) const;
    std::size_t read(Stream *, int = -1) const;
    std::size_t write(Stream *, int = -1) const;
    void shutdownRead() const;
    void shutdownWrite() const;
    IPEndpoint getLocalEndpoint() const;
    IPEndpoint getRemoteEndpoint() const;

private:
    int fd_;

    explicit TCPSocket(int);
};


TCPSocket::TCPSocket(TCPSocket &&other)
    : fd_(other.fd_)
{
    other.fd_ = -1;
}

} // namespace Gink
