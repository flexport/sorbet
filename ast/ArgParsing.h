#ifndef SORBET_AST_ARG_PARSING_H
#define SORBET_AST_ARG_PARSING_H
#include "ast/ast.h"
namespace sorbet::ast {
struct ParsedArg {
    core::LocOffsets loc;
    core::LocalVariable local;
    std::unique_ptr<ast::Expression> default_;
    core::ArgInfo::ArgFlags flags;
};
class ArgParsing {
public:
    static ParsedArg parseArg(std::unique_ptr<ast::Reference> arg);
    static std::vector<ParsedArg> parseArgs(ast::MethodDef::ARGS_store &args);
    static std::vector<u4> hashArgs(core::Context ctx, const std::vector<ParsedArg> &args);
};
}; // namespace sorbet::ast

#endif
