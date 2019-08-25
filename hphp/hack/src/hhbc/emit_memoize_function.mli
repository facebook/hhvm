(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Emit wrapper function for <<__Memoize>> function *)
val emit_wrapper_function :
  original_id:
    (* Original identifer for function, used for the wrapper *)
    Hhbc_id.Function.t ->
  renamed_id:
    (* Renamed identifier, used for the wrapped function *)
    Hhbc_id.Function.t ->
  is_method:bool ->
  deprecation_info:Typed_value.t list option ->
  (* Function definition in AST *)
  Tast.fun_ ->
  Hhas_function.t
