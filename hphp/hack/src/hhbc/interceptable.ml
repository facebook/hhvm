(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

let is_method_interceptable namespace ast_class original_id =
  let dynamic_invoke_functions =
    Hhbc_options.dynamic_invoke_functions !Hhbc_options.compiler_options in
  not (SSet.is_empty dynamic_invoke_functions) &&
    let class_id, _ = Hhbc_id.Class.elaborate_id namespace ast_class.Ast.c_name in
    let class_name = Hhbc_id.Class.to_unmangled_string class_id in
    let method_name = Hhbc_id.Method.to_raw_string original_id in
    let name = String.lowercase_ascii (class_name ^ "::" ^ method_name) in
    SSet.mem name dynamic_invoke_functions

let is_function_interceptable namespace ast_fun =
  let dynamic_invoke_functions =
    Hhbc_options.dynamic_invoke_functions !Hhbc_options.compiler_options in
  not (SSet.is_empty dynamic_invoke_functions) &&
    let fq_id, _ =
      Hhbc_id.Function.elaborate_id_with_builtins namespace ast_fun.Ast.f_name in
    let name = String.lowercase_ascii (Hhbc_id.Function.to_raw_string fq_id) in
    SSet.mem name (Hhbc_options.dynamic_invoke_functions !Hhbc_options.compiler_options)
