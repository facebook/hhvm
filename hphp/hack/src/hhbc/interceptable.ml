(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

let is_method_interceptable ast_class original_id =
  Hhbc_options.(
    let difs = dynamic_invoke_functions !compiler_options in
    jit_enable_rename_function !compiler_options
    || (not (SSet.is_empty difs))
       &&
       let class_id = Hhbc_id.Class.from_ast_name (snd ast_class.Aast.c_name) in
       let class_name = Hhbc_id.Class.to_unmangled_string class_id in
       let method_name = Hhbc_id.Method.to_raw_string original_id in
       let name = String.lowercase (class_name ^ "::" ^ method_name) in
       SSet.mem name difs)

let is_function_interceptable ast_fun =
  Hhbc_options.(
    let difs = dynamic_invoke_functions !compiler_options in
    (not (repo_authoritative !compiler_options))
    && jit_enable_rename_function !compiler_options
    || (not (SSet.is_empty difs))
       &&
       let name =
         Hhbc_id.Function.from_ast_name (snd ast_fun.Aast.f_name)
         |> Hhbc_id.Function.to_raw_string
         |> String.lowercase
       in
       SSet.mem name difs)
