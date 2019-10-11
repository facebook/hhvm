#!/bin/bash
set -u # terminate upon read of uninitalized variable
set -e # terminate upon non-zero-exit-codes (in case of pipe, only checks at end of pipe)
set -o pipefail # in a pipe, the whole pipe runs, but its exit code is that of the first failure
trap 'echo "exit code $? at line $LINENO" >&2' ERR

set -x # echo every statement in the script

# Try to get the path of this script relative to fbcode.
cd "$( dirname "${BASH_SOURCE[0]}" )/../../../.." >/dev/null
REGEN_COMMAND="$(realpath --relative-to=. "${BASH_SOURCE[0]}")"

# rustfmt is committed at fbsource/tools/third-party/rustfmt/rustfmt
RUSTFMT_PATH="$(realpath ../tools/third-party/rustfmt/rustfmt)"

SIGNER_PATH="$(realpath ../tools/signedsource)"

buck run hphp/hack/src/hh_oxidize --                                          \
  --out-dir hphp/hack/src/oxidized/gen                                        \
  --regen-command "$REGEN_COMMAND"                                            \
  --rustfmt-path "$RUSTFMT_PATH"                                              \
  hphp/hack/src/annotated_ast/aast_defs.ml                                    \
  hphp/hack/src/annotated_ast/aast.ml                                         \
  hphp/hack/src/ast/ast_defs.ml                                               \
  hphp/hack/src/ast/namespace_env.ml                                          \
  hphp/hack/src/decl/decl_defs.ml                                             \
  hphp/hack/src/decl/direct_decl_parser.ml                                    \
  hphp/hack/src/decl/shallow_decl_defs.ml                                     \
  hphp/hack/src/deps/fileInfo.ml                                              \
  hphp/hack/src/heap/ident.ml                                                 \
  hphp/hack/src/naming/nast.ml                                                \
  hphp/hack/src/options/globalOptions.ml                                      \
  hphp/hack/src/options/parserOptions.ml                                      \
  hphp/hack/src/options/typecheckerOptions.ml                                 \
  hphp/hack/src/typing/typing_defs.ml                                         \
  hphp/hack/src/typing/typing_reason.ml                                       \
  hphp/hack/src/utils/core/prim_defs.ml                                       \

buck run //hphp/hack/src/hh_codegen:hh_codegen --                             \
  --signer "$SIGNER_PATH"                                                     \
  --regen-cmd "$REGEN_COMMAND"                                                \
  --rustfmt "$RUSTFMT_PATH"                                                   \
  enum_constr                                                                 \
  --input "hphp/hack/src/oxidized/gen/aast.rs|crate::ast_defs|crate::aast::*" \
  --input "hphp/hack/src/oxidized/gen/aast_defs.rs|crate::aast_defs::*"       \
  --output "hphp/hack/src/oxidized/impl_gen/"                                 \

# Re-export generated modules (listed in gen/mod.rs) in the crate root, lib.rs
cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null
sed -i "/^pub use gen::/d" lib.rs
grep "^pub mod " gen/mod.rs | sed 's/^pub mod /pub use gen::/' >> lib.rs
