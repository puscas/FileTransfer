#pragma once

#include <filesystem>
#include <memory>

namespace FileTransfer
{
    class Server
    {
        private:
            struct Impl;
            std::unique_ptr<Impl> m_pimpl;

        public:

            Server();
            ~Server();

            const std::filesystem::path& workingDirectory() const;
            bool workingDirectory(const std::filesystem::path& folder);

            uint16_t port() const;
            bool port(uint16_t port);

            uint32_t address() const;
            bool address(uint32_t address);

            bool start();
            bool stop();

            bool is_running() const;
    };
}