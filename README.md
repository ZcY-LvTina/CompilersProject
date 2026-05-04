# 编译原理专题实践基础仓库

本仓库已切换为 C 语言开发骨架，按 `docs/` 中的三人分工和接口约定组织，目标是让小组成员可以直接并行开发并上传 GitHub 协作。

## 当前提供的内容

- 按 `common / lex / yacc / semantic / main` 划分的 C 源码骨架
- 统一的头文件接口，位于 `include/compiler_project/`
- 可运行的命令行程序 `compiler_project`
- 最小 `.l` / `.y` / 源程序样例
- 基于 `CTest` 的基础测试
- GitHub Actions CI
- 协作文档、接口文档、周报与会议纪要模板

## 快速开始

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/compiler_project check-layout
./build/compiler_project lex --spec samples/lex/minic.l --source samples/src/ok/demo.mc
./build/compiler_project yacc --spec samples/yacc/minic.y
./build/compiler_project semantic --lex-spec samples/lex/minic.l --yacc-spec samples/yacc/minic.y --source samples/src/ok/demo.mc
```

## 目录说明

```text
docs/        设计、接口、测试与汇报文档
include/     对外头文件与公共接口
samples/     词法、语法与源程序样例
src/         C 源码实现
tests/       CTest 测试代码
output/      运行输出目录
.github/     CI 与协作模板
```

## 协作建议

- A 同学主要改 `include/compiler_project/lex/` 和 `src/lex/`
- B 同学主要改 `include/compiler_project/yacc/` 和 `src/yacc/`
- C 同学主要改 `include/compiler_project/semantic/`、`include/compiler_project/common/`、`src/semantic/`、`src/common/`
- 所有人都可以补充 `tests/` 与 `docs/`
- 提交前至少运行一次 `ctest --test-dir build --output-on-failure`

## 上传到 GitHub

```bash
git add .
git commit -m "chore: initialize C project scaffold"
git branch -M main
git remote add origin <your-github-repo-url>
git push -u origin main
```
