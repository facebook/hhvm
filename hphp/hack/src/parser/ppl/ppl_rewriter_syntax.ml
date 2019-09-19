(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Core_kernel

(* Includes helpful builders *)
module CoroutineSyntax = Coroutine_syntax
open CoroutineSyntax

let ppl_infer_type_string = ["Infer"]

let ppl_infer_type_name_syntax =
  make_qualified_name_syntax ppl_infer_type_string ~has_leading:true

let ppl_macro_string = Coroutine_smart_constructor.ppl_macro_string

let receiver_string = "$__recv"

let receiver_variable_syntax = make_variable_expression_syntax receiver_string

let suspend_token_syntax = make_token_syntax TokenKind.Suspend

(* Special method names reserved by the Infer class *)
let reserved_method_names =
  ["sample"; "sample_model"; "factor"; "observe"; "condition"]

(* Determines whether a string is in the list of reserved method names *)
let is_infer_method method_string =
  List.mem ~equal:( = ) reserved_method_names method_string
