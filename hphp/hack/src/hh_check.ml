(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
 * This is a top level OCaml binary that immediately delegates to Rust main.
 * The reason to do it is to make OCaml runtime available to Rust, so we can use 
 * OCaml callbacks. This should also be possible by using Rust binary and OCaml
 * library compiled with -output-complete-obj flag, but I could not find a way 
 * to make it work in our build system.
 *)
external hh_check_main : unit -> unit = "hh_check_main"

let () =
  let () =
    Callback.register "print_tast_for_rust" Typing_ast_print.print_tast_for_rust;

    Typing_print_ffi.register_callbacks ()
  in
  hh_check_main ()
