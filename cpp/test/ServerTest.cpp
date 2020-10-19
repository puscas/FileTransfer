#include <gtest/gtest.h>
#include "Server.h"
#include "Client.h"
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>

namespace fs = std::filesystem;
using namespace FileTransfer;

using namespace std::literals::chrono_literals;

const auto HOME{fs::path(getenv("HOME"))};
const auto CURRENT{fs::current_path()};

TEST(TestServer, CurrentPath01) {
    Server server;
    ASSERT_EQ(CURRENT, server.workspace());
}

TEST(TestServer, CurrentPath02) {
    Server server;
    ASSERT_TRUE(server.workspace(HOME));
    ASSERT_NE(CURRENT, server.workspace());
}

TEST(TestServer, CurrentPath03) {
    Server server;
    ASSERT_TRUE(server.workspace(HOME));
    ASSERT_EQ(HOME, server.workspace());
}

TEST(TestServer, CurrentPath04) {
    Server server;
    const auto not_valid{fs::path("~")};
    ASSERT_FALSE(server.workspace(not_valid));
    ASSERT_EQ(CURRENT, server.workspace());
}

TEST(TestServer, CurrentPort01) {
    Server server;
    ASSERT_EQ(0, server.port());
}

TEST(TestServer, CurrentPort02) {
    Server server;
    ASSERT_TRUE(server.port(12000));
    ASSERT_EQ(12000, server.port());
}

TEST(TestServer, CurrentPort03) {
    Server server;
    ASSERT_EQ(0, server.port());
    ASSERT_TRUE(server.start());
    ASSERT_NE(0, server.port());
}

TEST(TestServer, CurrentPort04) {
    Server server;
    ASSERT_TRUE(server.start());
    ASSERT_NE(0, server.port());
    ASSERT_FALSE(server.port(server.port()));
}

TEST(TestServer, CurrentStartStop01) {
    Server server;
    ASSERT_FALSE(server.is_running());
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.is_running());
    ASSERT_FALSE(server.stop());
    ASSERT_FALSE(server.is_running());
}

TEST(TestServer, Address01) {
    Server server;
    ASSERT_TRUE(server.address(0));
}

TEST(TestServer, Address02) {
    Server server;
    ASSERT_FALSE(server.address(0xFFFFFFFF));
}

TEST(TestClient, ClientPath01) {
    Client client;
    const auto demo_file(std::filesystem::current_path() / "cpp/test/demo.jpg");
    ASSERT_TRUE(client.file(demo_file));
    ASSERT_EQ(demo_file, client.file());
}

TEST(TestSend, SendFile01) {
    const auto demo_file(std::filesystem::current_path() / "cpp/test/demo.jpg");
    const auto workspace("/tmp/");

    std::mutex m;
    std::condition_variable cv;

    Server server;
    std::thread([&]()
    {
        ASSERT_TRUE(server.workspace(workspace));
        ASSERT_TRUE(server.port(12345));
        ASSERT_TRUE(server.start());

        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&]
        {
            return !server.is_running();
        });
    }).detach();

    std::this_thread::sleep_for(100ms);

    Client client;
    ASSERT_TRUE(client.file(demo_file));
    ASSERT_TRUE(client.port(12345));
    ASSERT_TRUE(client.send());

    std::this_thread::sleep_for(100ms);
    
    ASSERT_FALSE(client.stop());
    ASSERT_FALSE(server.stop());
    cv.notify_all();
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}