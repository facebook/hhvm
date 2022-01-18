(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
external stc_main : unit -> unit = "stc_main"

type stc_ffi_print_tast_args = {
  opts: GlobalOptions.t;
  tast: Tast.program;
}

let stc_ffi_print_tast (args : stc_ffi_print_tast_args) : unit =
  let { opts; tast } = args in
  let backend = Provider_backend.Shared_memory in
  let ctx =
    Provider_context.empty_for_tool
      ~popt:opts
      ~tcopt:opts
      ~backend
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in
  Typing_ast_print.print_tast ctx tast

let () =
  Callback.register "stc_ffi_print_tast" stc_ffi_print_tast;
  stc_main ()
