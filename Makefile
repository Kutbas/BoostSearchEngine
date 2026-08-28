# 编译器设置
CXX = g++

# 编译选项：
# -std=c++17 : 使用 C++17 标准 (必须，因为用到了 std::filesystem)
# -Wall -Wextra : 开启所有常见警告，帮助写出更健壮的代码
# -O2 : 开启 O2 级别的代码优化，大幅提升文件读取和字符串处理的速度
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# 目标文件和源文件
TARGET = parser
SRC = parser.cc

# 默认目标
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 伪目标，用于清理编译产物
.PHONY: clean
clean:
	rm -f $(TARGET)