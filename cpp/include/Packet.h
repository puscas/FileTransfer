#include <cinttypes>

namespace FileTransfer
{
    static const uint32_t START_CODE {0x46555a45};

    enum PacketKind
    {
        UNKNOWN = 0,
        HEADER_SIZE = 1,
        HEADER_FILENAME = 2,
        HEADER_READY_FOR_DATA = 3,
        DATA = 4
    };

    struct __attribute__((packed, aligned(1))) Packet
    {
        const uint32_t start_code {START_CODE};
        uint8_t complete : 1;
        PacketKind kind : 7;
        uint64_t identifier{0};
        uint64_t index{0};
        uint64_t offset{0};
        uint16_t data_size{0};
        uint8_t data[1024]{0};
    };
}