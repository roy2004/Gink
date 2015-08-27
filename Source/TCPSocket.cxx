#include "TCPSocket.h"

#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>

#include <cstring>
#include <cerrno>
#include <cassert>

#include <Pixy/IO.h>

#include "ScopeGuard.h"
#include "GAIError.h"
#include "SystemError.h"
#include "Stream.h"


namespace Gink {

namespace {

void XGetAddrInfo(const char *, const char *, const ::addrinfo *, ::addrinfo **);
int XSocket(int, int, int);
void xsetsockopt(int, int, int, const void *, ::socklen_t);
void xbind(int, const ::sockaddr *, ::socklen_t);
void xlisten(int, int);
void XConnect(int, const ::sockaddr *, ::socklen_t, int);
int XAccept4(int, ::sockaddr *, ::socklen_t *, int, int);
::size_t XReadV(int, const ::iovec *, int, int);
::size_t XWrite(int, const void *, ::size_t, int);
void xshutdown(int, int);
void xgetsockname(int, ::sockaddr *, ::socklen_t *);
void xgetpeername(int, ::sockaddr *, ::socklen_t *);

} // namespace


TCPSocket
TCPSocket::Listen(const char *hostName, const char *serviceName, int backlog)
{
    ::addrinfo hints;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    ::addrinfo *result;
    ScopeGuard scopeGuard1([&result] { ::freeaddrinfo(result); });
    XGetAddrInfo(hostName, serviceName, &hints, &result);
    scopeGuard1.appoint();
    int fd;
    ScopeGuard scopeGuard2([&fd] { ::Close(fd); });
    fd = XSocket(result->ai_family, result->ai_socktype, result->ai_protocol);
    scopeGuard2.appoint();
    int onOff = 1;
    xsetsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &onOff, sizeof onOff);
    xsetsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &onOff, sizeof onOff);
    xbind(fd, result->ai_addr, result->ai_addrlen);
    xlisten(fd, backlog);
    TCPSocket instance(fd);
    scopeGuard2.dismiss();
    return instance;
}


TCPSocket
TCPSocket::Connect(const char *hostName, const char *serviceName, int timeout)
{
    ::addrinfo hints;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    ::addrinfo *result;
    ScopeGuard scopeGuard1([&result] { ::freeaddrinfo(result); });
    XGetAddrInfo(hostName, serviceName, &hints, &result);
    scopeGuard1.appoint();
    int fd;
    ScopeGuard scopeGuard2([&fd] { ::Close(fd); });
    fd = XSocket(result->ai_family, result->ai_socktype, result->ai_protocol);
    scopeGuard2.appoint();
    XConnect(fd, result->ai_addr, result->ai_addrlen, timeout);
    TCPSocket instance(fd);
    scopeGuard2.dismiss();
    return instance;
}


TCPSocket::TCPSocket(int fd)
    : fd_(fd)
{
}


TCPSocket::~TCPSocket()
{
    if (fd_ >= 0) {
        ::Close(fd_);
    }
}


TCPSocket
TCPSocket::accept(IPEndpoint *endpoint, int timeout) const
{
    int subFD;
    ScopeGuard scopeGuard([&subFD] { ::Close(subFD); });
    ::sockaddr_in name;
    ::socklen_t nameSize = sizeof name;
    subFD = XAccept4(fd_, reinterpret_cast<::sockaddr *>(&name), &nameSize, 0, timeout);
    scopeGuard.appoint();

    if (endpoint != nullptr) {
        endpoint->set(name);
    }

    TCPSocket instance(subFD);
    scopeGuard.dismiss();
    return instance;
}


std::size_t
TCPSocket::read(Stream *stream, int timeout) const
{
    assert(stream != nullptr);
    void *buffer1 = stream->getBuffer();
    std::size_t buffer1Size = stream->getBufferSize();
    static char buffer2[65536];

    ::iovec vector[2] = {
        {buffer1, buffer1Size},
        {buffer2, sizeof buffer2}
    };

    ::size_t numberOfBytes = XReadV(fd_, vector, 2, timeout);

    if (numberOfBytes == 0) {
        return 0;
    }

    if (numberOfBytes < buffer1Size) {
        stream->write(nullptr, numberOfBytes);
    } else {
        stream->write(nullptr, buffer1Size);
        stream->write(buffer2, numberOfBytes - buffer1Size);
    }

    return numberOfBytes;
}


std::size_t
TCPSocket::write(Stream *stream, int timeout) const
{
    assert(stream != nullptr);
    const void *data = stream->getData();
    std::size_t dataSize = stream->getDataSize();

    if (dataSize == 0) {
        return 0;
    }

    std::ptrdiff_t i = 0;

    do {
        i += XWrite(fd_, static_cast<const char *>(data) + i, dataSize - i, timeout);
    } while (i < static_cast<std::ptrdiff_t>(dataSize));

    stream->read(nullptr, i);
    return i;
}


void
TCPSocket::shutdownRead() const
{
    xshutdown(fd_, SHUT_RD);
}


void
TCPSocket::shutdownWrite() const
{
    xshutdown(fd_, SHUT_WR);
}


IPEndpoint
TCPSocket::getLocalEndpoint() const
{
    ::sockaddr_in name;
    ::socklen_t nameSize = sizeof name;
    xgetsockname(fd_, reinterpret_cast<::sockaddr *>(&name), &nameSize);
    return IPEndpoint(name);
}


IPEndpoint
TCPSocket::getRemoteEndpoint() const
{
    ::sockaddr_in name;
    ::socklen_t nameSize = sizeof name;
    xgetpeername(fd_, reinterpret_cast<::sockaddr *>(&name), &nameSize);
    return IPEndpoint(name);
}


namespace {

void
XGetAddrInfo(const char *hostName, const char *serviceName, const ::addrinfo *hints
             , ::addrinfo **result)
{
    int errorCode = ::GetAddrInfo(hostName, serviceName, hints, result);

    if (errorCode != 0) {
        throw GINK_GAI_ERROR(errorCode, "`::GetAddrInfo()` failed");
    }
}


int
XSocket(int domain, int type, int protocol)
{
    int fd = ::Socket(domain, type, protocol);

    if (fd < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::Socket()` failed");
    }

    return fd;
}


void
xsetsockopt(int sockfd, int level, int optname, const void *optval, ::socklen_t optlen)
{
    if (::setsockopt(sockfd, level, optname, optval, optlen) < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::setsockopt()` failed");
    }
}


void
xbind(int sockfd, const ::sockaddr *addr, ::socklen_t addrlen)
{
    if (::bind(sockfd, addr, addrlen) < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::bind()` failed");
    }
}


void
xlisten(int sockfd, int backlog)
{
    if (::listen(sockfd, backlog) < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::listen()` failed");
    }
}


void
XConnect(int fd, const ::sockaddr *name, ::socklen_t nameSize, int timeout)
{
    if (::Connect(fd, name, nameSize, timeout) < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::Connect()` failed");
    }
}


int
XAccept4(int fd, ::sockaddr *name, ::socklen_t *nameSize, int flags, int timeout)
{
    int subFD = ::Accept4(fd, name, nameSize, flags, timeout);

    if (subFD < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::Accept4()` failed");
    }

    return subFD;
}


::size_t
XReadV(int fd, const ::iovec *vector, int vectorLength, int timeout)
{
    ::ssize_t numberOfBytes = ::ReadV(fd, vector, vectorLength, timeout);

    if (numberOfBytes < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::ReadV()` failed");
    }

    return numberOfBytes;
}


::size_t
XWrite(int fd, const void *data, ::size_t dataSize, int timeout)
{
    ::ssize_t numberOfBytes = ::Write(fd, data, dataSize, timeout);

    if (numberOfBytes < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::Write()` failed");
    }

    return numberOfBytes;
}


void
xshutdown(int sockfd, int how)
{
    if (::shutdown(sockfd, how) < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::shutdown()` failed");
    }
}


void
xgetsockname(int sockfd, ::sockaddr *addr, ::socklen_t *addrlen)
{
    if (::getsockname(sockfd, addr, addrlen) < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::getsockname()` failed");
    }
}


void
xgetpeername(int sockfd, ::sockaddr *addr, ::socklen_t *addrlen)
{
    if (::getpeername(sockfd, addr, addrlen) < 0) {
        throw GINK_SYSTEM_ERROR(errno, "`::getpeername()` failed");
    }
}

} // namespace

} // namespace Gink
