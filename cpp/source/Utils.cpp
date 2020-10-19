#include "Utils.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stddef.h>

#include <fstream>

bool
FileTransfer::Utils::available_address(uint32_t address)
{
    if(address == 0)
    {
        return true;
    }
    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1)
    {
        return false;
    }
    for (auto it{ifaddr}; it != NULL; it = it->ifa_next)
    {
        if (it->ifa_addr == NULL)
        {
            continue;
        }            
        const auto family{it->ifa_addr->sa_family};
        if (family == AF_INET)
        {
            char host[NI_MAXHOST];
            const auto status{getnameinfo(
                it->ifa_addr,
                sizeof(struct sockaddr_in),
                host,
                NI_MAXHOST,
                NULL,
                0,
                NI_NUMERICHOST)};
            if (status == 0)
            {
                uint32_t nic_address{0};
                inet_pton(AF_INET, host, &nic_address);
                if(address == nic_address)
                {
                    return true;
                }
            }
        }
    }
    freeifaddrs(ifaddr);
    return false;
}

bool
FileTransfer::Utils::to_address(const std::string& string_address, uint32_t& address)
{
    const auto status{inet_pton(AF_INET, string_address.c_str(), &address)};
    return status == 1;
}

bool
FileTransfer::Utils::to_address(const uint32_t& address, std::string& string_address)
{
    if(address == 0)
    {
        string_address = "0.0.0.0";
        return true;
    }
    char host[INET_ADDRSTRLEN] {0};
    const auto status{inet_ntop(AF_INET, &address, host, INET_ADDRSTRLEN)};
    string_address = host;
    return status == host;
}

ssize_t
FileTransfer::Utils::file_size(std::fstream& stream)
{
    const auto offset{stream.tellg()};
    stream.ignore( std::numeric_limits<std::streamsize>::max() );
    const auto file_size{static_cast<ssize_t>(stream.gcount())};
    stream.clear();
    stream.seekg( offset, std::ios_base::beg);
    return file_size;
}