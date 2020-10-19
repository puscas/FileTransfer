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