#pragma once


#include <netinet/in.h>

#include <cstdint>


class IPEndpoint final
{
public:
    inline explicit IPEndpoint(const sockaddr_in &);
    inline IPEndpoint(const IPEndpoint &);
    inline void operator=(const IPEndpoint &);

    inline std::uint32_t getAddress() const;
    inline std::uint16_t getPortNumber() const;

private:
    std::uint32_t address_;
    std::uint16_t portNumber_;
};


IPEndpoint::IPEndpoint(const sockaddr_in &address)
    : address_(ntohl(address.sin_addr.s_addr)), portNumber_(ntohs(address.sin_port))
{
}


IPEndpoint::IPEndpoint(const IPEndpoint &other)
    : address_(other.address_), portNumber_(other.portNumber_)
{
}


void
IPEndpoint::operator=(const IPEndpoint &other)
{
    if (&other == this) {
        return;
    }

    address_ = other.address_;
    portNumber_ = other.portNumber_;
}


std::uint32_t
IPEndpoint::getAddress() const
{
    return address_;
}


std::uint16_t
IPEndpoint::getPortNumber() const
{
    return portNumber_;
}
