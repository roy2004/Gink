#include "TCPSocket.h"

#include <netdb.h>
#include <netinet/in.h>
#include <sys/uio.h>

#include <cstring>
#include <cerrno>

#include <Pixy/IO.h>

#include "GAIError.h"
#include "SystemError.h"
#include "ScopeGuard.h"
#include "Stream.h"


namespace {

int XSocket(int, int, int);
void XGetAddrInfo(const char *, const char *, const addrinfo *, addrinfo **);
void xsetsockopt(int, int, int, const void *, socklen_t);
void xbind(int, const sockaddr *, socklen_t);
void xlisten(int, int);
int XAccept4(int, sockaddr *, socklen_t *, int, int);
void XConnect(int, const sockaddr *, socklen_t, int);
size_t XRead(int, void *, size_t, int);
size_t XReadV(int, const iovec *, int, int);
size_t XWrite(int, const void *, size_t, int);

} // namespace


TCPSocket::TCPSocket()
    : fd_(XSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
{
}


TCPSocket::~TCPSocket()
{
    Close(fd_);
}


void
TCPSocket::listen(const char *hostName, const char *serviceName, int backlog)
{
    addrinfo hints;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo *result;
    ScopeGuard scopeGuard([&result] { freeaddrinfo(result); });
    XGetAddrInfo(hostName, serviceName, &hints, &result);
    scopeGuard.appoint();
    int reuseAddress = 1;
    xsetsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof reuseAddress);
    xbind(fd_, result->ai_addr, result->ai_addrlen);
    xlisten(fd_, backlog);
}


int
TCPSocket::accept(std::uint32_t *ipAddress, int *portNumber, int timeout)
{
    sockaddr_in address;
    socklen_t addressSize;

    int subFD = XAccept4(fd_, reinterpret_cast<sockaddr *>(&address), &addressSize, 0, timeout);

    if (ipAddress != nullptr) {
        *ipAddress = *reinterpret_cast<std::uint32_t *>(&address.sin_addr);
    }

    if (portNumber != nullptr) {
        *portNumber = ntohs(address.sin_port);
    }

    return subFD;
}


void
TCPSocket::connect(const char *hostName, const char *serviceName, int timeout)
{
    addrinfo hints;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo *result;
    ScopeGuard scopeGuard([&result] { freeaddrinfo(result); });
    XGetAddrInfo(hostName, serviceName, &hints, &result);
    scopeGuard.appoint();
    XConnect(fd_, result->ai_addr, result->ai_addrlen, timeout);
}


std::size_t
TCPSocket::read(Stream *stream, std::size_t minBufferSize, int timeout)
{
    assert(stream != nullptr);
    void *buffer1 = stream->getBuffer();
    std::size_t buffer1Size = stream->getBufferSize();
    size_t numberOfBytes;

    if (buffer1Size >= minBufferSize) {
        numberOfBytes = XRead(fd_, buffer1, buffer1Size, timeout);

        if (numberOfBytes > 0) {
            stream->write(nullptr, numberOfBytes);
        }
    } else {
        char *buffer2;
        ScopeGuard scopeGuard([&buffer2] { delete buffer2; });
        std::size_t buffer2Size = minBufferSize - buffer1Size;
        buffer2 = new char[buffer2Size];
        scopeGuard.appoint();

        iovec vector[2] = {
            {buffer1, buffer1Size},
            {buffer2, buffer2Size}
        };

        numberOfBytes = XReadV(fd_, vector, 2, timeout);

        if (numberOfBytes > 0) {
            if (numberOfBytes < buffer1Size) {
                stream->write(nullptr, numberOfBytes);
            } else {
                stream->write(nullptr, buffer1Size);
                stream->write(buffer2, numberOfBytes - buffer1Size);
            }
        }
    }

    return numberOfBytes;
}


std::size_t
TCPSocket::write(Stream *stream, int timeout)
{
    assert(stream != nullptr);
    const void *data = stream->getData();
    std::size_t dataSize = stream->getDataSize();

    if (dataSize == 0) {
        return 0;
    }

    std::ptrdiff_t i = 0;

    do {
        size_t n = XWrite(fd_, static_cast<const char *>(data) + i, dataSize - i, timeout);

        i += n;
    } while (i < static_cast<std::ptrdiff_t>(dataSize));

    stream->read(nullptr, i);
    return i;
}


namespace {

int
XSocket(int domain, int type, int protocol)
{
    int fd = Socket(domain, type, protocol);

    if (fd < 0) {
        throw SYSTEM_ERROR(errno, "`Socket()` failed");
    }

    return fd;
}


void
XGetAddrInfo(const char *hostName, const char *serviceName, const addrinfo *hints
             , addrinfo **result)
{
    int errorCode = GetAddrInfo(hostName, serviceName, hints, result);

    if (errorCode != 0) {
        throw GAI_ERROR(errorCode, "`GetAddrInfo()` failed");
    }
}


void
xsetsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    if (setsockopt(sockfd, level, optname, optval, optlen) < 0) {
        throw SYSTEM_ERROR(errno, "`xsetsockopt()` failed");
    }
}


void
xbind(int sockfd, const sockaddr *addr, socklen_t addrlen)
{
    if (bind(sockfd, addr, addrlen) < 0) {
        throw SYSTEM_ERROR(errno, "`bind()` failed");
    }
}


void
xlisten(int sockfd, int backlog)
{
    if (listen(sockfd, backlog) < 0) {
        throw SYSTEM_ERROR(errno, "`listen()` failed");
    }
}


int
XAccept4(int fd, sockaddr *address, socklen_t *addressSize, int flags, int timeout)
{
    int subFD = Accept4(fd, address, addressSize, flags, timeout);

    if (subFD < 0) {
        throw SYSTEM_ERROR(errno, "`Accept4()` failed");
    }

    return subFD;
}


void
XConnect(int fd, const sockaddr *address, socklen_t addressSize, int timeout)
{
    if (Connect(fd, address, addressSize, timeout) < 0) {
        throw SYSTEM_ERROR(errno, "`Connect()` failed");
    }
}


size_t
XRead(int fd, void *buffer, size_t bufferSize, int timeout)
{
    ssize_t numberOfBytes = Read(fd, buffer, bufferSize, timeout);

    if (numberOfBytes < 0) {
        throw SYSTEM_ERROR(errno, "`Read()` failed");
    }

    return numberOfBytes;
}


size_t
XReadV(int fd, const iovec *vector, int vectorLength, int timeout)
{
    ssize_t numberOfBytes = ReadV(fd, vector, vectorLength, timeout);

    if (numberOfBytes < 0) {
        throw SYSTEM_ERROR(errno, "`ReadV()` failed");
    }

    return numberOfBytes;
}


size_t
XWrite(int fd, const void *data, size_t dataSize, int timeout)
{
    ssize_t numberOfBytes = Write(fd, data, dataSize, timeout);

    if (numberOfBytes < 0) {
        throw SYSTEM_ERROR(errno, "`Write()` failed");
    }

    return numberOfBytes;
}

} // namespace
