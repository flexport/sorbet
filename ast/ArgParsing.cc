#include "ast/ArgParsing.h"
#include "common/typecase.h"
#include "core/Context.h"
#include "core/Hashing.h"

using namespace std;

namespace sorbet::ast {

namespace {
ParsedArg parseArg(const unique_ptr<ast::Reference> &arg) {
    ParsedArg parsedArg;

    typecase(
        arg.get(), [&](ast::UnresolvedIdent *nm) { Exception::raise("Unexpected unresolved name in arg!"); },
        [&](ast::RestArg *rest) {
            parsedArg = parseArg(rest->expr);
            parsedArg.flags.isRepeated = true;
        },
        [&](ast::KeywordArg *kw) {
            parsedArg = parseArg(kw->expr);
            parsedArg.flags.isKeyword = true;
        },
        [&](ast::OptionalArg *opt) {
            parsedArg = parseArg(opt->expr);
            parsedArg.flags.isDefault = true;
        },
        [&](ast::BlockArg *blk) {
            parsedArg = parseArg(blk->expr);
            parsedArg.flags.isBlock = true;
        },
        [&](ast::ShadowArg *shadow) {
            parsedArg = parseArg(shadow->expr);
            parsedArg.flags.isShadow = true;
        },
        [&](ast::Local *local) {
            parsedArg.local = local->localVariable;
            parsedArg.loc = local->loc;
        });

    return parsedArg;
}

unique_ptr<ast::Expression> getDefaultValue(unique_ptr<ast::Reference> arg) {
    unique_ptr<ast::Expression> default_;
    typecase(
        arg.get(), [&](ast::UnresolvedIdent *nm) { Exception::raise("Unexpected unresolved name in arg!"); },
        [&](ast::RestArg *rest) { default_ = getDefaultValue(move(rest->expr)); },
        [&](ast::KeywordArg *kw) { default_ = getDefaultValue(move(kw->expr)); },
        [&](ast::OptionalArg *opt) { default_ = getDefaultValue(move(opt->expr)); },
        [&](ast::BlockArg *blk) { default_ = getDefaultValue(move(blk->expr)); },
        [&](ast::ShadowArg *shadow) { default_ = getDefaultValue(move(shadow->expr)); },
        [&](ast::Local *local) {
            // No default.
        });
    return default_;
}

} // namespace

vector<ParsedArg> ArgParsing::parseArgs(const ast::MethodDef::ARGS_store &args) {
    vector<ParsedArg> parsedArgs;
    for (auto &arg : args) {
        auto *refExp = ast::cast_tree<ast::Reference>(arg.get());
        if (!refExp) {
            Exception::raise("Must be a reference!");
        }
        unique_ptr<ast::Reference> refExpImpl(refExp);
        parsedArgs.emplace_back(parseArg(refExpImpl));
        // Don't free the pointer; it's still owned by `args`.
        refExpImpl.release();
    }

    return parsedArgs;
}

std::vector<u4> ArgParsing::hashArgs(core::Context ctx, const std::vector<ParsedArg> &args) {
    std::vector<u4> result;
    result.reserve(args.size());
    for (const auto &e : args) {
        u4 arg = 0;
        u1 flags = 0;
        if (e.flags.isKeyword) {
            arg = core::mix(arg, core::_hash(e.local._name.data(ctx)->shortName(ctx)));
            flags += 1;
        }
        if (e.flags.isRepeated) {
            flags += 2;
        }
        if (e.flags.isDefault) {
            flags += 4;
        }
        if (e.flags.isShadow) {
            flags += 8;
        }
        if (e.flags.isBlock) {
            flags += 16;
        }

        result.push_back(core::mix(arg, flags));
    }
    return result;
}

unique_ptr<ast::Expression> ArgParsing::getDefault(const ParsedArg &parsedArg, unique_ptr<ast::Reference> arg) {
    if (!parsedArg.flags.isDefault) {
        return nullptr;
    }
    return getDefaultValue(move(arg));
}

} // namespace sorbet::ast
