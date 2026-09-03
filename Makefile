.PHONY: all clean

# 1. 编译器与现代编译选项
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# 2. 外部依赖库
JSON_LIB = -ljsoncpp
# 使用 -pthread 替代单纯的 -lpthread，全面开启多线程编译支持
THREAD_LIB = -pthread 

# 3. 三大目标产物定义
PARSER = parser
DEBUG = debug
HTTP_SERVER = http_server

all: $(PARSER) $(DEBUG) $(HTTP_SERVER)

# 目标 1：离线数据清洗与格式化工具 (C++17 std::filesystem 无需链接 boost)
$(PARSER): parser.cc
	$(CXX) -o $@ $^ $(CXXFLAGS)

# 目标 2：本地算法单测与排错交互终端
$(DEBUG): debug.cc
	$(CXX) -o $@ $^ $(CXXFLAGS) $(JSON_LIB)

# 目标 3：线上高并发 HTTP Web 服务端
$(HTTP_SERVER): http_server.cc
	$(CXX) -o $@ $^ $(CXXFLAGS) $(JSON_LIB) $(THREAD_LIB)

clean:
	rm -f $(PARSER) $(DEBUG) $(HTTP_SERVER)