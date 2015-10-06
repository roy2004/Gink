#pragma once


#include "TCPSocket.h"


namespace Gink {

class RPCServer
{
    RPCServer(const RPCServer &) = delete;
    void operator=RPCServer(const RPCServer &) = delete;

public:
    void run();

protected:
    inline RPCServer(const char *, const char *);
    inline virtual ~RPCServer();

private:
    TCPSocket tcpSocket_;
};


RPCServer::RPCServer(const char *hostName, const char *serviceName)
    : tcpSocket_(TCPSocket::Listen(hostName, serviceName))
{
}


RPCServer::~RPCServer()
{
}

} // namespace Gink
