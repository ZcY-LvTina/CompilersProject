# IR 规范

当前保留四元式接口：

`(op, arg1, arg2, result)`

语义组在 `src/semantic/ir_builder.c` 中实现三地址码生成逻辑，并将结果输出到 `output/ir/`。

## 当前支持的四元式

- `(ADD, a, b, t)`：加法。
- `(SUB, a, b, t)`：减法。
- `(MUL, a, b, t)`：乘法。
- `(DIV, a, b, t)`：除法。
- `(ASSIGN, src, _, dst)`：赋值。
- `(LABEL, _, _, L)`：标签。
- `(GOTO, _, _, L)`：无条件跳转。
- `(IF_LT, a, b, L)`：若 `a < b` 跳转。
- `(IF_LE, a, b, L)`：若 `a <= b` 跳转。
- `(IF_GT, a, b, L)`：若 `a > b` 跳转。
- `(IF_GE, a, b, L)`：若 `a >= b` 跳转。
- `(IF_EQ, a, b, L)`：若 `a == b` 跳转。
- `(IF_NE, a, b, L)`：若 `a != b` 跳转。
- `(PARAM, arg, _, _)`：函数调用实参入参。
- `(CALL, name, argc, result)`：调用函数 `name`，参数数量为 `argc`，返回值写入 `result`。
- `(FUNC_BEGIN, name, _, _)`：函数 IR 开始。
- `(FUNC_END, name, _, _)`：函数 IR 结束。
- `(RET, value, _, _)`：函数返回。

## 示例

```text
(100) (FUNC_BEGIN, inc, _, _)
(101) (ADD, x, 1, T1)
(102) (RET, T1, _, _)
(103) (FUNC_END, inc, _, _)
(104) (FUNC_BEGIN, main, _, _)
(111) (PARAM, sum, _, _)
(112) (CALL, inc, 1, T3)
(113) (ASSIGN, T3, _, sum)
(126) (FUNC_END, main, _, _)
```
