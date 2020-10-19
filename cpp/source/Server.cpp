#include "Packet.h"
#include "Server.h"
#include "Utils.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <fstream>

#include <ifaddrs.h>

using namespace FileTransfer;
using namespace boost;
using namespace boost::asio;

namespace
{
    struct File
    {
        bool flag_end_filename{false};
        bool flag_end_filesize{false};
        ssize_t filesize{0};
        ssize_t processed{0};
        std::map<uint64_t, std::string> filename;
        std::fstream stream;
    };

    template <typename N> 
    bool address_in_use(uint16_t port)
    {
        io_service service;
        ip::tcp::acceptor acceptor(service);

        boost::system::error_code error_code;
        acceptor.open(N::v4(), error_code) || acceptor.bind({ N::v4(), port }, error_code);

        return error_code == error::address_in_use;
    }

    void
    process_packet(File& file, const Packet& packet)
    {
        switch(packet.kind)
        {
            case PacketKind::HEADER_FILENAME:
            {
                file.filename[packet.offset] = std::string(reinterpret_cast<const char*>(packet.data), packet.data_size);
                file.flag_end_filename |= packet.complete;
            }
            break;
            case PacketKind::HEADER_SIZE:
            {
                file.filesize = *reinterpret_cast<const ssize_t*>(packet.data);
                file.flag_end_filesize |= packet.complete;
            }
            break;
            case PacketKind::DATA:
            {
                file.stream.write(reinterpret_cast<const char*>(&packet.data[0]), packet.data_size);
                file.processed += packet.data_size;
                if(file.processed == file.filesize)
                {
                    file.stream.close();
                }
            }
            break;
            case PacketKind::UNKNOWN:
            default: {}
            break;
        }
    }

    bool
    prepare_to_receive_data(const std::filesystem::path& working_directory, File& file)
    {
        if(file.flag_end_filename && file.flag_end_filesize && !file.stream.is_open())
        {
            std::stringstream stream;
            for(auto& element: file.filename)
            {
                const auto& string{std::get<1>(element)};
                stream << string;
            }
            stream.seekg(0, std::ios::end);
            if(stream.tellg() >= static_cast<ssize_t>(std::get<0>(*file.filename.rbegin())))
            {
                stream.seekg(0, std::ios::beg);

                auto filename{stream.str()};
                filename.erase(std::remove( filename.begin(), filename.end(), '\"' ), filename.end());
                const auto full_path{std::filesystem::path(working_directory / filename)};

                std::ofstream(full_path.c_str(), std::ofstream::app | std::fstream::binary);
                file.stream.open(full_path.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);
                if (file.stream)
                {
                    return true;
                }

                std::cout << "Can't use the output file! -> " << full_path.c_str() << " | " << std::strerror(errno) << std::endl;
            }
        }
        return false;
    }

    void
    call_for_data(ip::tcp::socket& socket, uint64_t identifier, uint64_t offset)
    {
        Packet packet;
        packet.kind = PacketKind::HEADER_READY_FOR_DATA;
        packet.data_size = 0;
        packet.complete = 1;
        packet.identifier = identifier;
        packet.offset = offset;
        asio::write(socket, asio::buffer(reinterpret_cast<void*>(&packet), sizeof(packet)));
    }
}

namespace FileTransfer
{
    class ServerConnection
    {
        private:
            ip::tcp::socket m_socket;
            Packet m_packet;
            const std::filesystem::path& m_workspace;
            File m_file;

            void
            data_received(size_t read, const boost::system::error_code& error_code)
            {
                if(read == sizeof(m_packet))
                {
                    if(m_packet.start_code == START_CODE)
                    {
                        process_packet(m_file, m_packet);
                        if(prepare_to_receive_data(m_workspace, m_file))
                        {
                            call_for_data(m_socket, m_packet.identifier, Utils::file_size(m_file.stream));
                        }
                    }
                }
                if(error_code)
                {
                    boost::system::error_code close_error_code;
                    m_socket.shutdown(ip::tcp::socket::shutdown_both, close_error_code);
                    m_socket.close();
                }
                else
                {
                    prepare_async_read();
                }
            }

        public:

            ServerConnection(
                asio::io_service& io_service,
                const std::filesystem::path& working_directory) : 
                m_socket(io_service),
                m_workspace(working_directory)
            {
            }

            ~ServerConnection()
            {
            }

            void
            prepare_async_read()
            {
                asio::async_read(
                    m_socket,
                    asio::buffer(reinterpret_cast<void*>(&m_packet), sizeof(m_packet)),
                    boost::bind(
                        &ServerConnection::data_received,
                        this,
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error));
            }

            ip::tcp::socket&
            socket()
            {
                return m_socket;
            }
    };
}

struct Server::Impl
{
    bool m_running{false};
    uint16_t m_port{0};
    uint32_t m_address{0};
    std::filesystem::path m_workspace;

    asio::io_service m_io_service;

    std::unique_ptr<ip::tcp::acceptor> m_acceptor_uptr;
    std::list<std::shared_ptr<ServerConnection>> m_connections;

    Impl() : m_workspace{std::filesystem::current_path()}
    {
    }

    ~Impl()
    {
        stop();
    }

    const std::filesystem::path&
    workspace() const
    {
        return m_workspace;
    }

    bool
    workspace(const std::filesystem::path& folder)
    {
        if(std::filesystem::exists(folder))
        {
            m_workspace = folder;
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
        if(!address_in_use<ip::tcp>(port) && !m_running)
        {
            m_port = port;
            return true;
        }
        return false;
    }

    uint32_t
    address() const
    {
        return m_address;
    }

    bool
    address(uint32_t address)
    {
        if(Utils::available_address(address))
        {
            m_address = address;
            return true;
        }
        return false;
    }

    bool
    start()
    {
        if(!m_running)
        {
            std::string string_address;
            assert(FileTransfer::Utils::to_address(m_address, string_address));
            m_acceptor_uptr = std::make_unique<ip::tcp::acceptor>(m_io_service, ip::tcp::endpoint( ip::address::from_string(string_address), m_port ));
            m_port = m_acceptor_uptr->local_endpoint().port();
            start_accept();

            m_running = true;
            std::thread([&]{ m_io_service.run(); }).detach();
            return m_running;
        }
        return false;
    }

    bool
    stop()
    {
        m_running = false;
        m_io_service.stop();
        return m_running;
    }

    bool
    is_running() const
    {
        return m_running;
    }

    void
    handle_accept(std::shared_ptr<ServerConnection> connection_sptr, const boost::system::error_code& error_code)
    {
        if (!error_code)
        {
            connection_sptr->prepare_async_read();
        }
        start_accept();
    }

    void
    start_accept()
    {
        auto connection_sptr{m_connections.emplace_back(std::make_shared<ServerConnection>(m_io_service, m_workspace))};
        m_acceptor_uptr->async_accept(
            connection_sptr->socket(),
            boost::bind(
                &Impl::handle_accept,
                this,
                connection_sptr,
                asio::placeholders::error));
    }
};

Server::Server() : m_pimpl{std::make_unique<Impl>()}
{
}

Server::~Server()
{
    stop();
}

const std::filesystem::path&
Server::workspace() const
{
    return m_pimpl->workspace();
}

bool
Server::workspace(const std::filesystem::path& folder)
{
    return m_pimpl->workspace(folder);
}

uint16_t
Server::port() const
{
    return m_pimpl->port();
}

bool
Server::port(uint16_t port)
{
    return m_pimpl->port(port);
}

uint32_t
Server::address() const
{
    return m_pimpl->address();
}

bool
Server::address(uint32_t address)
{
    return m_pimpl->address(address);
}

bool
Server::start()
{
    return m_pimpl->start();
}

bool
Server::stop()
{
    return m_pimpl->stop();
}

bool
Server::is_running() const
{
    return m_pimpl->is_running();
}