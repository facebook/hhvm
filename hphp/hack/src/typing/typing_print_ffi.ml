(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_defs_core

let empty_env =
  let ctx =
    Provider_context.empty_for_tool
      ~popt:ParserOptions.default
      ~tcopt:TypecheckerOptions.default
      ~backend:Provider_backend.Shared_memory
  in
  Typing_env.empty ctx Relative_path.default ~droot:None

let typing_print_ffi_debug (ty : locl_ty) = Typing_print.debug empty_env ty

let typing_print_ffi_debug_i (ty : internal_type) =
  Typing_print.debug_i empty_env ty

let register_callbacks () =
  Callback.register "typing_print_ffi_debug" typing_print_ffi_debug;
  Callback.register "typing_print_ffi_debug_i" typing_print_ffi_debug_i
