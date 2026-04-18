# IR 规范

当前保留四元式接口：

`(op, arg1, arg2, result)`

语义组后续在 `src/semantic/ir_builder.c` 中实现三地址码生成逻辑，并将结果输出到 `output/ir/`。
