#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#include "Client.h"
#include "Server.h"
#include "Utils.h"

namespace
{
    enum class Mode
    {
        UNKNOWN,
        SERVER,
        CLIENT
    };

    struct Context
    {
        Mode mode{Mode::UNKNOWN};
        uint16_t port{0};
        uint32_t address{0};
        uint64_t bandwidth{0};
        std::string stage;
    };

    void print_context(const Context& context)
    {
        std::stringstream stream;
        stream << "MODE=" << (context.mode == Mode::SERVER ? "SERVER" : 
                             (context.mode == Mode::CLIENT ? "CLIENT" : "UNKNOWN")) << std::endl
               << "PORT=" << context.port << std::endl
               << "STAGE=" << context.stage << std::endl;
    }
}

 
int main(int argc, char *argv[])
{
    using namespace std::chrono_literals;

    std::cout << "## File Transfer ##" << std::endl;

    if(argc < 5)
    {
        std::cout << "Invalid arguments!" << std::endl << std::endl;
        std::cout << "Commands::" << std::endl;
        std::cout << "-s : Set program to run as a server." << std::endl;
        std::cout << "-c : Set program to run as a client." << std::endl;
        std::cout << "-p : Port to use as a server or port to connect as a client." << std::endl;
        std::cout << "-a : Address to use as a server or address to connect as a client." << std::endl;
        std::cout << "-f : Folder to store the assets when used as a server. Full path to the file to send as a client." << std::endl;
        std::cout << "-b : Maximum bandwidth to use as a client. Bandwidth is in Bps and the minimum is 1024 Bps." << std::endl << std::endl;
        std::cout << "Server example:: ./FileTransfer -s -p 12000 -f /home/user/Desktop/out/ -a 192.168.1.136" << std::endl;
        std::cout << "Client example:: ./FileTransfer -c -p 12000 -f /home/user/Desktop/demo.jpg -a 192.168.1.136" << std::endl;
        return 1;
    }

    Context context;
    for (int count{ 0 }; count < argc; ++count)
    {
        std::string argument(argv[count]);
        if(argument.compare("-c") == 0)
        {
            context.mode = Mode::CLIENT;
        }
        else if(argument.compare("-s") == 0)
        {
            context.mode = Mode::SERVER;
        }
        else if(argument.compare("-p") == 0)
        {
            if(++count < argc )
            {
                context.port = std::stoi(argv[count]);
            }
        }
        else if(argument.compare("-a") == 0)
        {
            if(++count < argc )
            {
                uint32_t address{0};
                if(FileTransfer::Utils::to_address(argv[count], address))
                {
                    context.address = address;
                }
            }
        }
        else if(argument.compare("-f") == 0)
        {
            if(++count < argc )
            {
                context.stage = argv[count];
            }
        }
        else if(argument.compare("-b") == 0)
        {
            if(++count < argc )
            {
                context.bandwidth = std::stoull(argv[count]);
            }
        }
    }

    print_context(context);

    switch(context.mode)
    {
        case Mode::SERVER:
        {
            FileTransfer::Server server;
            if(!server.port(context.port))
            {
                std::cout << "Port not available " << context.port << ". Used " << server.port() << std::endl;
            }
            if(!server.address(context.address))
            {
                std::cout << "IP not available " << context.address << ". Used " << server.address() << std::endl;
            }
            server.workspace(context.stage);
            server.start();
            std::cout << "Running in server mode. Used port " << server.port() << " and working on folder " << server.workspace() <<  std::endl;

            std::cout << "Press 'Enter' to stop" << std::endl;
            getchar();

            server.stop();
        }
        break;
        case Mode::CLIENT:
        {
            FileTransfer::Client client;
            client.port(context.port);
            client.address(context.address);
            client.bandwidth(context.bandwidth);
            client.file(context.stage);
            client.send();

            std::cout << "Running in client mode. Used port " << client.port() << " and copied file " << client.file() <<  std::endl;

            client.stop();
        }
        break;
        case Mode::UNKNOWN:
        default:
        {
            std::cout << "Invalid arguments.";
        }
        break;
    }
    return 0;
}