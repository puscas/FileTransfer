#include <stdint.h>
#include <string>

namespace FileTransfer
{
    namespace Utils
    {
        bool available_address(uint32_t address);
        bool to_address(const std::string& string_address, uint32_t& address);
        bool to_address(const uint32_t& address, std::string& string_address);

        ssize_t file_size(std::fstream& stream);
    }
}