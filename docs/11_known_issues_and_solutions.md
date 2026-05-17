# 问题记录与解决方法

更新时间：2026-05-17

本文档记录开发过程中已经遇到的问题、定位过程、解决方法和仍需跟进的风险，方便新开对话或组员接手时继续推进。

## 1. LR 表升级后统一流水线栈溢出

现象：

- `ctest` 可通过，但执行统一输出命令时进程退出状态为 `139`。
- 命令示例：

```bash
./build/compiler_project --lex samples/lex/minic.l --yacc samples/yacc/minic.y --input samples/src/ok/demo.mc --out output
```

定位：

- 使用 AddressSanitizer 单独构建 `build-asan` 后复现。
- ASan 报告 `cp_build_lr_table` 入口处 stack-overflow。
- 根因是 LR(1)/LALR 表状态数变多，`LRTableResult` 和构造临时对象体积较大，作为局部变量时压垮调用栈。

解决：

- 将 `cp_build_lr_table` 内大型结果和临时结构改为静态存储。
- 将 `cp_parse_tokens` 内 LR 表结果改为静态存储。
- 修复后普通构建和 ASan 构建均可通过统一输出命令。

验证：

```bash
cmake --build build
ctest --test-dir build --output-on-failure
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
./build/compiler_project --lex samples/lex/minic.l --yacc samples/yacc/minic.y --input samples/src/ok/demo.mc --out output
```

## 2. LALR 同 core 合并计数不正确

现象：

- LR(1) 状态合并到 LALR 状态时，同 core 判断可能失真。
- 风险是 reduce lookahead 合并不完整，导致分析表状态数或 action 填写异常。

定位：

- `core_count` 原逻辑调用 `core_item_exists(set, current_item)`，当前 item 在自身集合里必然存在，导致计数条件错误。

解决：

- 改为只在当前 item 之前扫描，统计唯一 `(production_id, dot)` 对。

验证：

- `--build-table samples/yacc/minic.y` 可生成 89 个 LALR 状态。
- 控制流样例可 parse、type check、IR 生成。

## 3. 语法分析 trace 行被截断

现象：

- `output/log/parse_steps.txt` 中较深状态栈的 trace 行缺少 action 尾部。

定位：

- `ParseTrace.steps` 已扩到 256 字节，但 `trace_action` 内部临时 `line` 缓冲仍为 128 字节。

解决：

- 将 `state_text / symbol_text / line` 缓冲扩容。
- trace 输出当前包含 `states / symbols / lookahead / action`。

验证：

```bash
sed -n '9,14p' output/log/parse_steps.txt
```

## 4. dangling else 冲突处理

现象：

- `if (a) if (b) ... else ...` 属于经典 dangling else 场景，会出现 shift/reduce 冲突。

处理策略：

- 当前冲突消解在无显式优先级时采用 shift 优先。
- 效果是 `else` 绑定最近的 `if`，符合常见 C 语言行为。

后续建议：

- 在报告中明确写出该策略。
- 可进一步用文法拆分 `matched_stmt / unmatched_stmt` 来做无冲突文法，但实现成本更高。

## 5. 函数调用只检查数量，不检查参数类型

现象：

- 早期 `cp_check_function_call` 只检查实参数量。
- `inc(void_value)` 这类调用不会被准确拦截。

解决：

- 函数调用检查扩展为：
- 校验被调用标识符是否为函数。
- 校验实参数量是否匹配。
- 对每个实参综合类型，并检查是否可赋给对应形参类型。

验证：

- 新增测试 `test_function_call_argument_type_error`。
- 参数类型不匹配时报错 `8009 function argument type mismatch`。

## 6. 当前仍需跟进的问题

待处理：

- 函数前向调用：当前类型检查按 AST 顺序入表，函数需要先定义后调用；若要支持先调用后定义，需要全局预扫描函数签名。
- `void` 变量声明：当前类型系统可识别 `void`，但语言规则还没有禁止 `void v;` 这种变量声明。
- 短路布尔：尚未支持 `&& / || / !`，需要词法、文法、AST、类型检查、IR 同步扩展。
- 错误恢复：当前遇到语法错误直接失败，尚未做 panic-mode 或同步 token 恢复。
- LR 表可视化：当前输出 action/goto，尚未输出完整 item 集与状态迁移图。
