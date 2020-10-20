#include "Client.h"
#include "Packet.h"
#include "Utils.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <fstream>
#include <boost/thread.hpp>

using namespace FileTransfer;

struct Client::Impl
{
    uint16_t m_port{0};
    uint32_t m_address{0};
    uint64_t m_identifier{0};
    std::filesystem::path m_file;

    boost::thread_group m_thread_group;
    boost::asio::io_service m_io_service;

    uint64_t m_bandwidth{0};
    uint64_t m_sleep{0};

    Impl()
    {
    }

    ~Impl()
    {
    }

    const std::filesystem::path&
    file() const
    {
        return m_file;
    }

    bool
    file(const std::filesystem::path& file_path)
    {
        if(std::filesystem::exists(file_path))
        {
            m_file = file_path;
            return true;
        }
        return false;
    }

    uint16_t
    port() const
    {
        return m_port;
    }

    bool
    port(uint16_t port)
    {
        m_port = port;
        return true;
    }

    uint32_t
    address() const
    {
        return m_address;
    }

    bool
    address(uint32_t address)
    {
        m_address = address;
        return true;
    }

    bool
    send()
    {
        std::string address;
        if(FileTransfer::Utils::to_address(m_address, address) && std::filesystem::exists(m_file))
        {
            boost::asio::ip::tcp::socket socket(m_io_service);
            boost::asio::ip::tcp::resolver resolver(m_io_service);
            boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(boost::asio::ip::tcp::resolver::query(address, std::to_string(m_port)));
            boost::asio::connect(socket, endpoint);

            // Read file
            std::fstream file;
            file.open (m_file.c_str(), std::ios::in | std::ios::binary);

            uint64_t identifier{m_identifier++};
            size_t count{0};

            // Create the first packet with the file size
            const auto file_size{Utils::file_size(file)};
            Packet header;
            header.index = count++;
            header.kind = PacketKind::HEADER_SIZE;
            header.identifier = identifier;
            header.data_size = sizeof(file_size);
            auto data{reinterpret_cast<ssize_t*>(header.data)};
            *data = file_size;
            header.complete = 1;
            socket.send(boost::asio::buffer(reinterpret_cast<void*>(&header), sizeof(header)));

            // Create the next packet(s) with the filename
            std::stringstream stream;
            stream << m_file.filename();
            const auto maximum{sizeof(Packet::data)};
            ssize_t read{0};
            uint64_t offset{0};
            do
            {
                Packet header;
                header.index = count++;
                header.kind = PacketKind::HEADER_FILENAME;
                header.identifier = identifier;
                header.offset = offset;
                stream.read(reinterpret_cast<char*>(header.data), maximum); 
                read = stream.gcount();
                assert(read >= 0);
                header.data_size = static_cast<uint16_t>(read);
                header.complete = (read == maximum ? 0 : 1);
                socket.send(boost::asio::buffer(reinterpret_cast<void*>(&header), sizeof(header)));
                offset += header.data_size;
            }
            while ( read == maximum );

            boost::asio::streambuf stream_buffer;
            boost::system::error_code error_code;
            Packet header_signal;
            const auto read_size{socket.read_some(boost::asio::buffer(reinterpret_cast<void*>(&header_signal), sizeof(header_signal)))};

            if (read_size == sizeof(header_signal))
            {
                // Send now the content of the file, from the specified offset.
                offset = header_signal.offset;
                stream.seekg( offset, std::ios_base::beg);

                std::cout << "Sending file from offset " << offset << std::endl;

                do
                {
                    count++;
                    Packet data;
                    data.index = count++;
                    data.kind = PacketKind::DATA;
                    data.identifier = identifier;
                    data.offset = offset;
                    file.read(reinterpret_cast<char*>(data.data), maximum); 
                    read = file.gcount();
                    assert(read >= 0);
                    data.data_size = static_cast<uint16_t>(read);
                    header.complete = (read == maximum ? 0 : 1);
                    socket.send(boost::asio::buffer(reinterpret_cast<void*>(&data), sizeof(data)));
                    offset += header.data_size;

                    if(m_sleep)
                    {
                        std::this_thread::sleep_for(std::chrono::nanoseconds(m_sleep));
                    }
                }
                while ( read == maximum );

                assert(file.eof());
                return true;
            }
        }
        return false;
    }

    bool
    stop()
    {
        return false;
    }

    bool
    is_running() const
    {
        return false;
    }

    uint64_t
    bandwidth() const
    {
        return m_bandwidth;
    }

    bool
    bandwidth(uint64_t bandwidth)
    {
        const uint64_t minimum{1024};
        if(bandwidth >= minimum)
        {
            m_bandwidth = bandwidth;
            m_sleep = (minimum * 1'000'000'000) / m_bandwidth;
            return true;
        }
        return false;
    }
};

Client::Client() : m_pimpl{std::make_unique<Impl>()}
{
}

Client::~Client()
{
    stop();
}

const std::filesystem::path&
Client::file() const
{
    return m_pimpl->file();
}

bool
Client::file(const std::filesystem::path& file_path)
{
    return m_pimpl->file(file_path);
}

uint16_t
Client::port() const
{
    return m_pimpl->port();
}

bool
Client::port(uint16_t port)
{
    return m_pimpl->port(port);
}

uint32_t
Client::address() const
{
    return m_pimpl->address();
}

bool
Client::address(uint32_t address)
{
    return m_pimpl->address(address);
}

bool
Client::send()
{
    return m_pimpl->send();
}

bool
Client::stop()
{
    return m_pimpl->stop();
}

bool
Client::is_running() const
{
    return m_pimpl->is_running();
}

uint64_t
Client::bandwidth() const
{
    return m_pimpl->bandwidth();
}

bool
Client::bandwidth(uint64_t bandwidth)
{
    return m_pimpl->bandwidth(bandwidth);
}
