#include <gtest/gtest.h>
#include "Server.h"
#include <filesystem>

namespace fs = std::filesystem;
using namespace FileTransfer;

const auto HOME{fs::path(getenv("HOME"))};
const auto CURRENT{fs::current_path()};

TEST(TestServer, CurrentPath01) {
    Server server;
    ASSERT_EQ(CURRENT, server.workingDirectory());
}

TEST(TestServer, CurrentPath02) {
    Server server;
    ASSERT_TRUE(server.workingDirectory(HOME));
    ASSERT_NE(CURRENT, server.workingDirectory());
}

TEST(TestServer, CurrentPath03) {
    Server server;
    ASSERT_TRUE(server.workingDirectory(HOME));
    ASSERT_EQ(HOME, server.workingDirectory());
}

TEST(TestServer, CurrentPath04) {
    Server server;
    const auto not_valid{fs::path("~")};
    ASSERT_FALSE(server.workingDirectory(not_valid));
    ASSERT_EQ(CURRENT, server.workingDirectory());
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

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}