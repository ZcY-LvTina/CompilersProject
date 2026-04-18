# 接口规范

## A -> B

统一通过 `TokenStream` 交互，字段定义位于 `include/compiler_project/common/token.h`。

## B -> C

主接口为 `ASTNode`，辅接口可扩展为规约日志。

## C -> 全组

公共数据结构由 `include/compiler_project/common/` 提供：

- `Token`
- `ASTNode`
- `SymbolEntry`
- `TypeExpr`
- `Quadruple`
- `CompilerError`
- `Result[T]`
