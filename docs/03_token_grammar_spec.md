# Token 与文法约定

## Token 五元组

`(token_type, lexeme, line, column, attr)`

## 当前最小支持

- 关键字：`int` `return`
- 标识符：`ID`
- 整数字面量：`NUM`
- 运算符：`=` `+`
- 分隔符：`;`

## 文法最小子集

当前样例仅覆盖简单赋值声明，后续可扩展到表达式、函数与控制流。

