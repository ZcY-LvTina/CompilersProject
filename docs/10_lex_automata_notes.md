# A 模块说明：Lex / 自动机 / 词法分析

## 完成内容

- `.l` 文件解析：读取规则名、正则模式、token kind、`skip` 标记和规则优先级。
- 正则检查：支持普通字符、转义字符、字符类、连接、`|`、`*`、`+`、`?`、括号。
- 自动机构造：使用 Thompson 构造生成 NFA，合并多条规则 NFA，确定化为 DFA，并执行 DFA 最小化。
- 词法运行时：基于最小 DFA 扫描源程序，执行最长匹配；匹配长度相同时使用 `.l` 文件中靠前规则优先。
- Token 输出：稳定输出 `TokenStream`，供 B 的 Yacc 模块使用。

## 当前 token 子集

当前最小 Mini-C 子集沿用项目样例命名：

```text
INT RETURN ID NUM ASSIGN PLUS SEMI EOF
```

说明：

- `NUM` 表示整数字面量，暂不改为 `INT_CONST`，以保持 `samples/yacc/minic.y` 兼容。
- 空白符规则输出为 `WS`，但带有 `skip` 标记，不进入最终 token 流。
- 关键字和标识符冲突遵守最长匹配：`int` 为 `INT`，`intx` 为 `ID`。

## A -> B 接口

B 只依赖 `include/compiler_project/common/token.h` 中的 `TokenStream`：

```text
(kind, lexeme, line, column, attr)
```

A 不要求 B 读取 NFA、DFA 或正则内部结构。EOF token 总是输出。

## 错误码

```text
1000-1999  .l 文件解析错误
2000-2999  正则 / 自动机构造错误
5000-5999  源程序词法错误
```

常见错误：

- `1002`：规则缺少正则模式。
- `1003`：规则缺少 token kind。
- `2001`：正则中存在悬空转义符。
- `2003`：字符类未闭合。
- `5001`：源程序中出现无法匹配的字符。

## 验证命令

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/compiler_project lex --spec samples/lex/minic.l --source samples/src/ok/demo.mc
```

当前环境验证记录：

- `cmake -S . -B build`：失败，CMake 在编译器探测阶段无法删除/生成 `CMakeFiles/CMakeScratch` 临时文件，报 `Access is denied`。
- `gcc -std=c17 ... tests/test_scaffold.c ...`：通过。
- `.\build-a-lex-test.exe`：通过。
- `.\compiler_project-a-lex.exe lex --spec samples\lex\minic.l --source samples\src\ok\demo.mc`：通过。

`demo.mc` 的词法输出应包含：

```text
(INT, "int", 1, 1, null)
(ID, "sum", 1, 5, null)
(ASSIGN, "=", 1, 9, null)
(NUM, "12", 1, 11, value=12)
(SEMI, ";", 1, 13, null)
(EOF, "", 3, 1, null)
```
