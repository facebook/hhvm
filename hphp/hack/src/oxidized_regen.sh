#!/bin/bash
set -u # terminate upon read of uninitalized variable
set -o pipefail # in a pipe, the whole pipe runs, but its exit code is that of the first failure

CYAN=""
WHITE=""
BOLD=""
RESET=""

# If these fail that's just fine.
if [ -t 1 ]; then
    CYAN=$(tput setaf 6)
    WHITE=$(tput setaf 7)
    BOLD=$(tput bold)
    RESET=$(tput sgr0)
fi

set -e # terminate upon non-zero-exit-codes (in case of pipe, only checks at end of pipe)
trap 'echo "exit code $? at line $LINENO" >&2' ERR

function summary {
    echo -e "\n$BOLD$CYAN==>$WHITE ${1}$RESET"
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
  --copy-types-file hphp/hack/src/oxidized/copy_types.txt                     \
  hphp/hack/src/annotated_ast/aast_defs.ml                                    \
  hphp/hack/src/annotated_ast/namespace_env.ml                                \
  hphp/hack/src/ast/ast_defs.ml                                               \
  hphp/hack/src/custom_error/custom_error.ml                                  \
  hphp/hack/src/custom_error/custom_error_config.ml                           \
  hphp/hack/src/custom_error/error_message.ml                                 \
  hphp/hack/src/custom_error/patt_binding_ty.ml                               \
  hphp/hack/src/custom_error/patt_error.ml                                    \
  hphp/hack/src/custom_error/patt_locl_ty.ml                                  \
  hphp/hack/src/custom_error/patt_name.ml                                     \
  hphp/hack/src/custom_error/patt_string.ml                                   \
  hphp/hack/src/custom_error/patt_var.ml                                      \
  hphp/hack/src/custom_error/validation_err.ml                                \
  hphp/hack/src/decl/decl_defs.ml                                             \
  hphp/hack/src/decl/pos/pos_or_decl.ml                                       \
  hphp/hack/src/decl/shallow_decl_defs.ml                                     \
  hphp/hack/src/errors/user_error.ml                                          \
  hphp/hack/src/errors/errors.ml                                              \
  hphp/hack/src/errors/error_codes.ml                                         \
  hphp/hack/src/errors/message.ml                                             \
  hphp/hack/src/naming/name_context.ml                                        \
  hphp/hack/src/naming/naming_error.ml                                        \
  hphp/hack/src/typing/nast_check/nast_check_error.ml                         \
  hphp/hack/src/parser/parsing_error.ml                                       \
  hphp/hack/src/errors/quickfix.ml                                            \
  hphp/hack/src/naming/naming_phase_error.ml                                  \
  hphp/hack/src/options/declParserOptions.ml                                  \
  hphp/hack/src/options/globalOptions.ml                                      \
  hphp/hack/src/options/parserOptions.ml                                      \
  hphp/hack/src/options/saved_state_rollouts.ml                               \
  hphp/hack/src/options/typecheckerOptions.ml                                 \
  hphp/hack/src/package/package.ml                                            \
  hphp/hack/src/package/packageInfo.ml                                        \
  hphp/hack/src/parser/full_fidelity_parser_env.ml                            \
  hphp/hack/src/search/utils/searchTypes.ml                                   \
  hphp/hack/src/typing/service/reason_collector.ml                            \
  hphp/hack/src/typing/service/tast_hashes.ml                                 \
  hphp/hack/src/typing/service/type_counter.ml                                \
  hphp/hack/src/typing/service/map_reduce_ffi.ml                              \
  hphp/hack/src/typing/tast.ml                                                \
  hphp/hack/src/typing/tast_with_dynamic.ml                                                \
  hphp/hack/src/typing/type_parameter_env.ml                                  \
  hphp/hack/src/typing/typing_defs_core.ml                                    \
  hphp/hack/src/typing/typing_defs.ml                                         \
  hphp/hack/src/typing/typing_inference_env.ml                                \
  hphp/hack/src/typing/typing_kinding_defs.ml                                 \
  hphp/hack/src/typing/typing_reason.ml                                       \
  hphp/hack/src/typing/typing_tyvar_occurrences.ml                            \
  hphp/hack/src/typing/xhp_attribute.ml                            \
  hphp/hack/src/utils/decl_reference.ml                                       \
  hphp/hack/src/parser/scoured_comments.ml                                    \

# Add exports in oxidized/lib.rs from oxidized/gen/mod.rs.
# BSD sed doesn't have -i
sed "/^pub use gen::/d" hphp/hack/src/oxidized/lib.rs > hphp/hack/src/oxidized/lib.rs.tmp
mv hphp/hack/src/oxidized/lib.rs.tmp hphp/hack/src/oxidized/lib.rs
grep "^pub mod " hphp/hack/src/oxidized/gen/mod.rs | sed 's/^pub mod /pub use gen::/' >> hphp/hack/src/oxidized/lib.rs

summary "Write individually-converted oxidized files"
"${BUILD_AND_RUN}" src/hh_oxidize hh_oxidize --regen-command "$REGEN_COMMAND" --rustfmt-path "$RUSTFMT_PATH" hphp/hack/src/deps/fileInfo.ml > hphp/hack/src/deps/rust/file_info.rs
"${BUILD_AND_RUN}" src/hh_oxidize hh_oxidize --regen-command "$REGEN_COMMAND" --rustfmt-path "$RUSTFMT_PATH" hphp/hack/src/utils/core/prim_defs.ml > hphp/hack/src/deps/rust/prim_defs.rs
"${BUILD_AND_RUN}" src/hh_oxidize hh_oxidize --regen-command "$REGEN_COMMAND" --rustfmt-path "$RUSTFMT_PATH" hphp/hack/src/naming/naming_types.ml > hphp/hack/src/naming/rust/naming_types.rs
"${BUILD_AND_RUN}" src/hh_oxidize hh_oxidize --regen-command "$REGEN_COMMAND" --rustfmt-path "$RUSTFMT_PATH" hphp/hack/src/lints/lints_core.ml > hphp/hack/src/utils/lint/lint.rs
"${BUILD_AND_RUN}" src/hh_oxidize hh_oxidize --regen-command "$REGEN_COMMAND" --rustfmt-path "$RUSTFMT_PATH" hphp/hack/src/typing/hh_distc_types.ml > hphp/hack/src/typing/hh_distc_types/hh_distc_types.rs

summary "Write oxidized/impl_gen/"
"${BUILD_AND_RUN}" src/hh_codegen hh_codegen                                  \
  --regen-cmd "$REGEN_COMMAND"                                                \
  --rustfmt "$RUSTFMT_PATH"                                                   \
  enum-helpers                                                                \
  --input "hphp/hack/src/oxidized/gen/aast_defs.rs|crate::aast_defs::*|crate::ast_defs|crate::LocalIdMap" \
  --input "hphp/hack/src/oxidized/gen/ast_defs.rs|crate::ast_defs::*"         \
  --output "hphp/hack/src/oxidized/impl_gen/"                                 \

summary "Write oxidized/aast_visitor/"
"${BUILD_AND_RUN}" src/hh_codegen hh_codegen                                  \
  --regen-cmd "$REGEN_COMMAND"                                                \
  --rustfmt "$RUSTFMT_PATH"                                                   \
  visitor                                                                     \
  --input "hphp/hack/src/oxidized/gen/aast_defs.rs"                           \
  --input "hphp/hack/src/oxidized/gen/ast_defs.rs"                            \
  --output "hphp/hack/src/oxidized/aast_visitor/"                             \
  --root "Program"                                                            \

summary "Write oxidized/asts"
"${BUILD_AND_RUN}" src/hh_codegen hh_codegen                                  \
  --regen-cmd "$REGEN_COMMAND"                                                \
  --rustfmt "$RUSTFMT_PATH"                                                   \
  asts                                                                        \
  --input "hphp/hack/src/oxidized/gen/aast_defs.rs"                           \
  --input "hphp/hack/src/oxidized/gen/ast_defs.rs"                            \
  --output "hphp/hack/src/oxidized/asts/"                                     \
  --root "Program"                                                            \

summary "Write elab/transform/"
"${BUILD_AND_RUN}" src/hh_codegen hh_codegen                                  \
  --regen-cmd "$REGEN_COMMAND"                                                \
  --rustfmt "$RUSTFMT_PATH"                                                   \
  elab-transform                                                              \
  --input "hphp/hack/src/oxidized/gen/aast_defs.rs"                           \
  --input "hphp/hack/src/oxidized/gen/ast_defs.rs"                            \
  --output "hphp/hack/src/elab/"                                              \
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
  hphp/hack/src/annotated_ast/namespace_env.ml                                \
  hphp/hack/src/ast/ast_defs.ml                                               \
  hphp/hack/src/custom_error/custom_error.ml                                  \
  hphp/hack/src/custom_error/custom_error_config.ml                           \
  hphp/hack/src/custom_error/error_message.ml                                 \
  hphp/hack/src/custom_error/patt_binding_ty.ml                               \
  hphp/hack/src/custom_error/patt_error.ml                                    \
  hphp/hack/src/custom_error/patt_locl_ty.ml                                  \
  hphp/hack/src/custom_error/patt_name.ml                                     \
  hphp/hack/src/custom_error/patt_string.ml                                   \
  hphp/hack/src/custom_error/patt_var.ml                                      \
  hphp/hack/src/custom_error/validation_err.ml                                \
  hphp/hack/src/decl/decl_defs.ml                                             \
  hphp/hack/src/decl/pos/pos_or_decl.ml                                       \
  hphp/hack/src/decl/shallow_decl_defs.ml                                     \
  hphp/hack/src/deps/fileInfo.ml                                              \
  hphp/hack/src/naming/naming_types.ml                                        \
  hphp/hack/src/naming/nast.ml                                                \
  hphp/hack/src/options/saved_state_rollouts.ml                               \
  hphp/hack/src/package/package.ml                                            \
  hphp/hack/src/package/packageInfo.ml                                        \
  hphp/hack/src/parser/scoured_comments.ml                                    \
  hphp/hack/src/typing/typing_defs_core.ml                                    \
  hphp/hack/src/typing/typing_defs.ml                                         \
  hphp/hack/src/typing/typing_reason.ml                                       \
  hphp/hack/src/typing/xhp_attribute.ml                                       \
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
  --input "hphp/hack/src/oxidized_by_ref/gen/ast_defs.rs"                     \
  --input "hphp/hack/src/oxidized_by_ref/gen/shallow_decl_defs.rs"            \
  --input "hphp/hack/src/oxidized_by_ref/gen/typing_defs_core.rs"             \
  --input "hphp/hack/src/oxidized_by_ref/gen/typing_defs.rs"                  \
  --input "hphp/hack/src/oxidized_by_ref/gen/typing_reason.rs"                \
  --input "hphp/hack/src/oxidized_by_ref/gen/xhp_attribute.rs"                \
  --input "hphp/hack/src/oxidized_by_ref/manual/direct_decl_parser.rs"        \
  --input "hphp/hack/src/oxidized_by_ref/manual/t_shape_map.rs"               \
  --extern-input "hphp/hack/src/oxidized/gen/ast_defs.rs"                     \
  --extern-input "hphp/hack/src/oxidized/gen/typing_defs.rs"                  \
  --extern-input "hphp/hack/src/oxidized/gen/typing_defs_core.rs"             \
  --extern-input "hphp/hack/src/oxidized/gen/typing_reason.rs"                \
  --extern-input "hphp/hack/src/oxidized/gen/xhp_attribute.rs"                \
  --output "hphp/hack/src/oxidized_by_ref/decl_visitor/"                      \
  --root "Decls"                                                              \
