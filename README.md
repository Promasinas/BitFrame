# BitFream

C/C++ 高性能计算系统框架。

## 项目结构

```
BitFream/
├── src/
│   ├── MemoryManagement/    # 内存管理子系统
│   ├── OperatorKernals/     # 算子内核
│   ├── RuntimeManagement/   # 运行时管理
│   ├── LanguageWrapper/     # 语言绑定 (Python/Lua)
│   ├── API/                 # 公共 API
│   └── Utils/               # 通用工具
├── include/                 # 全局头文件
├── lib/                     # 第三方库
│   └── log.c/               # rxi/log.c 轻量日志库
├── test/                    # 单元测试
│   └── log/                 # log.c 测试
├── build/                   # 构建输出
├── bin/                     # 可执行文件
└── docs/                    # 文档
```

## 构建

```bash
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## 测试

```bash
# 运行所有测试
cd build && ctest

# 或直接运行
./test/build/test_log.exe
```

## 许可证

MIT
