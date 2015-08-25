#pragma once


#include <netinet/in.h>

#include <cstdint>


namespace Gink {

struct IPEndpoint
{
    inline explicit IPEndpoint();
    inline explicit IPEndpoint(const ::sockaddr_in &);

    inline void set(const ::sockaddr_in &);

    std::uint32_t address;
    std::uint16_t portNumber;
};


IPEndpoint::IPEndpoint()
    : address(0), portNumber(0)
{
}


IPEndpoint::IPEndpoint(const ::sockaddr_in &name)
    : address(ntohl(name.sin_addr.s_addr)), portNumber(ntohs(name.sin_port))
{
}


void
IPEndpoint::set(const ::sockaddr_in &name)
{
    address = ntohl(name.sin_addr.s_addr);
    portNumber = ntohs(name.sin_port);
}

} // namespace Gink
