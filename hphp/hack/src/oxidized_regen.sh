#!/bin/bash
set -u # terminate upon read of uninitalized variable
set -e # terminate upon non-zero-exit-codes (in case of pipe, only checks at end of pipe)
set -o pipefail # in a pipe, the whole pipe runs, but its exit code is that of the first failure
trap 'echo "exit code $? at line $LINENO" >&2' ERR

CYAN=""
WHITE=""
BOLD=""
RESET=""

if [ -t 1 ]; then
    CYAN=$(tput setaf 6)
    WHITE=$(tput setaf 7)
    BOLD=$(tput bold)
    RESET=$(tput sgr0)
fi

function summary {
    echo -e "$BOLD$CYAN==>$WHITE ${1}$RESET"
}

# Try to get the path of this script relative to fbcode/.
#set -x # echo every statement in the script

FBCODE_ROOT="$(dirname "${BASH_SOURCE[0]}")/../../.."
REGEN_COMMAND="$(realpath --relative-to="${FBCODE_ROOT}" "${BASH_SOURCE[0]}")"
cd "$FBCODE_ROOT"

# rustfmt is committed at fbsource/tools/third-party/rustfmt/rustfmt
RUSTFMT_PATH="${RUSTFMT_PATH:-"$(realpath ../tools/third-party/rustfmt/rustfmt)"}"

BUILD_AND_RUN="hphp/hack/scripts/build_and_run.sh"

summary "Write oxidized/gen/"
"${BUILD_AND_RUN}" src/hh_oxidize hh_oxidize                                  \
  --out-dir hphp/hack/src/oxidized/gen                                        \
  --regen-command "$REGEN_COMMAND"                                            \
  --rustfmt-path "$RUSTFMT_PATH"                                              \
  hphp/hack/src/annotated_ast/aast_defs.ml                                    \
  hphp/hack/src/annotated_ast/aast.ml                                         \
  hphp/hack/src/annotated_ast/namespace_env.ml                                \
  hphp/hack/src/ast/ast_defs.ml                                               \
  hphp/hack/src/decl/pos/pos_or_decl.ml                                       \
  hphp/hack/src/deps/fileInfo.ml                                              \
  hphp/hack/src/deps/typing_deps_mode.ml                                      \
  hphp/hack/src/errors/errors.ml                                              \
  hphp/hack/src/errors/error_codes.ml                                         \
  hphp/hack/src/naming/naming_types.ml                                        \
  hphp/hack/src/naming/nast.ml                                                \
  hphp/hack/src/options/globalOptions.ml                                      \
  hphp/hack/src/options/parserOptions.ml                                      \
  hphp/hack/src/options/typecheckerOptions.ml                                 \
  hphp/hack/src/parser/full_fidelity_parser_env.ml                            \
  hphp/hack/src/utils/core/prim_defs.ml                                       \
  hphp/hack/src/utils/decl_reference.ml                                       \
  hphp/hack/src/parser/scoured_comments.ml                                    \

# Add exports in oxidized/lib.rs from oxidized/gen/mod.rs.
# BSD sed doesn't have -i
sed "/^pub use gen::/d" hphp/hack/src/oxidized/lib.rs > hphp/hack/src/oxidized/lib.rs.tmp
mv hphp/hack/src/oxidized/lib.rs.tmp hphp/hack/src/oxidized/lib.rs
grep "^pub mod " hphp/hack/src/oxidized/gen/mod.rs | sed 's/^pub mod /pub use gen::/' >> hphp/hack/src/oxidized/lib.rs

summary "Write oxidized/impl_gen/"
"${BUILD_AND_RUN}" src/hh_codegen hh_codegen                                  \
  --regen-cmd "$REGEN_COMMAND"                                                \
  --rustfmt "$RUSTFMT_PATH"                                                   \
  enum-helpers                                                                \
  --input "hphp/hack/src/oxidized/gen/aast.rs|crate::ast_defs|crate::aast::*|crate::LocalIdMap" \
  --input "hphp/hack/src/oxidized/gen/aast_defs.rs|crate::aast_defs::*"       \
  --input "hphp/hack/src/oxidized/gen/ast_defs.rs|crate::ast_defs::*"         \
  --output "hphp/hack/src/oxidized/impl_gen/"                                 \

summary "Write oxidized/aast_visitor/"
"${BUILD_AND_RUN}" src/hh_codegen hh_codegen                                  \
  --regen-cmd "$REGEN_COMMAND"                                                \
  --rustfmt "$RUSTFMT_PATH"                                                   \
  visitor                                                                     \
  --input "hphp/hack/src/oxidized/gen/aast.rs"                                \
  --input "hphp/hack/src/oxidized/gen/aast_defs.rs"                           \
  --input "hphp/hack/src/oxidized/gen/ast_defs.rs"                            \
  --input "hphp/hack/src/oxidized/manual/doc_comment.rs"                      \
  --output "hphp/hack/src/oxidized/aast_visitor/"                             \
  --root "Program"                                                            \

summary "Write oxidized_by_ref/gen/"
"${BUILD_AND_RUN}" src/hh_oxidize hh_oxidize                                  \
  --out-dir hphp/hack/src/oxidized_by_ref/gen                                 \
  --regen-command "$REGEN_COMMAND"                                            \
  --rustfmt-path "$RUSTFMT_PATH"                                              \
  --extern-types-file hphp/hack/src/oxidized_by_ref/extern_types.txt          \
  --owned-types-file hphp/hack/src/oxidized_by_ref/owned_types.txt            \
  --copy-types-file hphp/hack/src/oxidized_by_ref/copy_types.txt              \
  --by-ref                                                                    \
  hphp/hack/src/annotated_ast/aast_defs.ml                                    \
  hphp/hack/src/annotated_ast/aast.ml                                         \
  hphp/hack/src/annotated_ast/namespace_env.ml                                \
  hphp/hack/src/ast/ast_defs.ml                                               \
  hphp/hack/src/decl/decl_defs.ml                                             \
  hphp/hack/src/decl/pos/pos_or_decl.ml                                       \
  hphp/hack/src/decl/shallow_decl_defs.ml                                     \
  hphp/hack/src/deps/fileInfo.ml                                              \
  hphp/hack/src/errors/error_codes.ml                                         \
  hphp/hack/src/errors/errors.ml                                              \
  hphp/hack/src/naming/naming_types.ml                                        \
  hphp/hack/src/naming/nast.ml                                                \
  hphp/hack/src/options/declParserOptions.ml                                  \
  hphp/hack/src/options/globalOptions.ml                                      \
  hphp/hack/src/options/parserOptions.ml                                      \
  hphp/hack/src/options/typecheckerOptions.ml                                 \
  hphp/hack/src/parser/scoured_comments.ml                                    \
  hphp/hack/src/typing/tast.ml                                                \
  hphp/hack/src/typing/type_parameter_env.ml                                  \
  hphp/hack/src/typing/typing_cont_key.ml                                     \
  hphp/hack/src/typing/typing_defs_core.ml                                    \
  hphp/hack/src/typing/typing_defs.ml                                         \
  hphp/hack/src/typing/typing_env_return_info.ml                              \
  hphp/hack/src/typing/typing_env_types.ml                                    \
  hphp/hack/src/typing/typing_fake_members.ml                                 \
  hphp/hack/src/typing/typing_inference_env.ml                                \
  hphp/hack/src/typing/typing_kinding_defs.ml                                 \
  hphp/hack/src/typing/typing_local_types.ml                                  \
  hphp/hack/src/typing/typing_per_cont_env.ml                                 \
  hphp/hack/src/typing/typing_reason.ml                                       \
  hphp/hack/src/typing/typing_tyvar_occurrences.ml                            \
  hphp/hack/src/utils/decl_reference.ml                                       \
  hphp/hack/src/utils/core/prim_defs.ml                                       \

# Add exports in oxidized_by_ref/lib.rs from oxidized_by_ref/gen/mod.rs.
# BSD sed doesn't have -i
sed "/^pub use gen::/d" hphp/hack/src/oxidized_by_ref/lib.rs > hphp/hack/src/oxidized_by_ref/lib.rs.tmp
mv hphp/hack/src/oxidized_by_ref/lib.rs.tmp hphp/hack/src/oxidized_by_ref/lib.rs
grep "^pub mod " hphp/hack/src/oxidized_by_ref/gen/mod.rs | sed 's/^pub mod /pub use gen::/' >> hphp/hack/src/oxidized_by_ref/lib.rs

summary "Write oxidized_by_ref/decl_visitor/"
"${BUILD_AND_RUN}" src/hh_codegen hh_codegen                                  \
  --regen-cmd "$REGEN_COMMAND"                                                \
  --rustfmt "$RUSTFMT_PATH"                                                   \
  by-ref-decl-visitor                                                         \
  --input "hphp/hack/src/oxidized_by_ref/gen/aast_defs.rs"                    \
  --input "hphp/hack/src/oxidized_by_ref/gen/ast_defs.rs"                     \
  --input "hphp/hack/src/oxidized_by_ref/gen/shallow_decl_defs.rs"            \
  --input "hphp/hack/src/oxidized_by_ref/gen/typing_defs_core.rs"             \
  --input "hphp/hack/src/oxidized_by_ref/gen/typing_defs.rs"                  \
  --input "hphp/hack/src/oxidized_by_ref/gen/typing_reason.rs"                \
  --input "hphp/hack/src/oxidized_by_ref/manual/direct_decl_parser.rs"        \
  --input "hphp/hack/src/oxidized_by_ref/manual/t_shape_map.rs"               \
  --extern-input "hphp/hack/src/oxidized/gen/aast_defs.rs"                    \
  --extern-input "hphp/hack/src/oxidized/gen/ast_defs.rs"                     \
  --output "hphp/hack/src/oxidized_by_ref/decl_visitor/"                      \
  --root "Decls"                                                              \

summary "Write oxidized_by_ref/nast_visitor/"
"${BUILD_AND_RUN}" src/hh_codegen hh_codegen                                  \
  --regen-cmd "$REGEN_COMMAND"                                                \
  --rustfmt "$RUSTFMT_PATH"                                                   \
  by-ref-nast-visitor                                                         \
  --input "hphp/hack/src/oxidized_by_ref/gen/aast.rs"                         \
  --input "hphp/hack/src/oxidized_by_ref/gen/aast_defs.rs"                    \
  --input "hphp/hack/src/oxidized_by_ref/gen/ast_defs.rs"                     \
  --input "hphp/hack/src/oxidized_by_ref/gen/namespace_env.rs"                \
  --input "hphp/hack/src/oxidized_by_ref/manual/doc_comment.rs"               \
  --extern-input "hphp/hack/src/oxidized/gen/aast_defs.rs"                    \
  --extern-input "hphp/hack/src/oxidized/gen/aast.rs"                         \
  --extern-input "hphp/hack/src/oxidized/gen/ast_defs.rs"                     \
  --output "hphp/hack/src/oxidized_by_ref/nast_visitor/"                      \
  --root "Program"                                                            \
