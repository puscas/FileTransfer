PROJECT := FileTransfer

INCLUDE_DIR := include
SOURCE_DIR := source
OBJECT_DIR := objects
TESTS_DIR := test


CXX := g++ -std=c++17 -I $(INCLUDE_DIR) -I /usr/include -O0 -g
CXXFLAGS := -c -Wall -DASIO_STANDALONE -pthread
LDFFLAGS := -lboost_system -pthread -lboost_thread
TEST_LDFLAGS := -lgtest -lgtest_main -L /usr/lib -L /usr/lib/x86_64-linux-gnu


SRCS := $(wildcard $(SOURCE_DIR)/*.cpp) $(wildcard $(TESTS_DIR)/*.cpp)
OBJS := $(patsubst $(SOURCE_DIR)/%.cpp,$(OBJECT_DIR)/%.o, $(patsubst $(TESTS_DIR)/%.cpp,$(OBJECT_DIR)/%.o, $(SRCS)))
OBJS_WO_MAIN := $(filter-out %/main.o, $(OBJS))
OBJS_WO_TESTS := $(filter-out %Test.o, $(OBJS))


$(PROJECT): $(OBJS)
	$(CXX) -o ../$@ $(OBJS_WO_TESTS) $(LDFFLAGS)
	$(CXX) -o ../$@Test $(OBJS_WO_MAIN) $(LDFFLAGS) $(TEST_LDFLAGS)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CXX) -o $@ $(CXXFLAGS) $<
$(OBJECT_DIR)/%.o: $(TESTS_DIR)/%.cpp
	$(CXX) -o $@ $(CXXFLAGS) $<

$(OBJS): | $(OBJECT_DIR)

$(OBJECT_DIR):
	mkdir -p $(OBJECT_DIR)

all: $(PROJECT)

clean:
	rm -f $(OBJECT_DIR)/*.o 

