(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* Emit wrapper function for <<__Memoize>> function *)
val emit_wrapper_function :
  (* Original identifer for function, used for the wrapper *)
  original_id: Hhbc_id.Function.t ->
  (* Renamed identifier, used for the wrapped function *)
  renamed_id: Hhbc_id.Function.t ->
  is_method: bool ->
  deprecation_info: (Typed_value.t list) option ->
  (* Function definition in AST *)
  Ast.fun_ ->
  Hhas_function.t
