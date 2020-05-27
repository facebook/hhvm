#!/bin/bash
set -u # terminate upon read of uninitalized variable
set -e # terminate upon non-zero-exit-codes (in case of pipe, only checks at end of pipe)
set -o pipefail # in a pipe, the whole pipe runs, but its exit code is that of the first failure
trap 'echo "exit code $? at line $LINENO" >&2' ERR

set -x # echo every statement in the script

# Try to get the path of this script relative to fbcode/.
FBCODE_ROOT="$(dirname "${BASH_SOURCE[0]}")/../../../.."
REGEN_COMMAND="$(realpath --relative-to="${FBCODE_ROOT}" "${BASH_SOURCE[0]}")"
cd "$FBCODE_ROOT"

# rustfmt is committed at fbsource/tools/third-party/rustfmt/rustfmt
RUSTFMT_PATH="${RUSTFMT_PATH:-"$(realpath ../tools/third-party/rustfmt/rustfmt)"}"

BUILD_AND_RUN="hphp/hack/scripts/build_and_run.sh"
"${BUILD_AND_RUN}" src/hh_oxidize hh_oxidize                                  \
  --out-dir hphp/hack/src/oxidized_by_rc/gen                                 \
  --regen-command "$REGEN_COMMAND"                                            \
  --rustfmt-path "$RUSTFMT_PATH"                                              \
  --extern-types-file hphp/hack/src/oxidized_by_rc/extern_types.txt          \
  --by-rc                                                                    \
  hphp/hack/src/annotated_ast/aast_defs.ml                                    \
  hphp/hack/src/annotated_ast/aast.ml                                         \
  hphp/hack/src/annotated_ast/namespace_env.ml                                \
  hphp/hack/src/ast/ast_defs.ml                                               \
  hphp/hack/src/decl/decl_defs.ml                                             \
  hphp/hack/src/decl/direct_decl_parser.ml                                    \
  hphp/hack/src/decl/shallow_decl_defs.ml                                     \
  hphp/hack/src/deps/fileInfo.ml                                              \
  hphp/hack/src/errors/errors.ml                                              \
  hphp/hack/src/errors/error_codes.ml                                         \
  hphp/hack/src/naming/naming_types.ml                                        \
  hphp/hack/src/naming/nast.ml                                                \
  hphp/hack/src/typing/typing_defs.ml                                         \
  hphp/hack/src/typing/typing_defs_core.ml                                    \
  hphp/hack/src/utils/core/prim_defs.ml                                       \
  hphp/hack/src/parser/scoured_comments.ml                                    \

# Re-export generated modules (listed in gen/mod.rs) in the crate root, lib.rs
cd "$(dirname "${REGEN_COMMAND}")"
# BSD sed doesn't have -i
sed "/^pub use gen::/d" lib.rs > lib.rs.tmp
mv lib.rs.tmp lib.rs
grep "^pub mod " gen/mod.rs | sed 's/^pub mod /pub use gen::/' >> lib.rs
