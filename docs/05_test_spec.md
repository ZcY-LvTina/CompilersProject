# 测试规范

- `tests/`：当前统一使用 `test_scaffold.c` 承载基础测试，后续可拆为多文件
- 推荐按模块继续扩展 `lex / yacc / semantic / integration`

提交前要求：

1. 运行 `ctest --test-dir build --output-on-failure`
2. 新功能至少补 1 个测试
3. Bug 修复必须补回归测试
