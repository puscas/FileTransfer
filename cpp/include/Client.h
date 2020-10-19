#pragma once

#include <filesystem>
#include <memory>

namespace FileTransfer
{
    class Client
    {
        private:
            struct Impl;
            std::unique_ptr<Impl> m_pimpl;

        public:

            Client();
            ~Client();

            const std::filesystem::path& file() const;
            bool file(const std::filesystem::path& file_path);

            uint16_t port() const;
            bool port(uint16_t port);

            uint32_t address() const;
            bool address(uint32_t address);

            bool send();
            bool stop();

            bool is_running() const;
    };
}