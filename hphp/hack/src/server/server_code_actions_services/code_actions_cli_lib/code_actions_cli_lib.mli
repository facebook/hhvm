(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
- Enables previewing code actions from the command line sans language server.
- Can `failwith` a pretty-printed Lsp.Error.t
- Currently used only by hh_single_type_check
*)
val run_exn :
  Provider_context.t ->
  Provider_context.entry ->
  Ide_api_types.range ->
  title_prefix:string ->
  unit
