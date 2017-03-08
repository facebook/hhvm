(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open Core
open Instruction_sequence
open Hhbc_from_nast

module A = Ast

type t = {
  method_attributes    : Hhas_attribute.t list;
  method_is_protected  : bool;
  method_is_public     : bool;
  method_is_private    : bool;
  method_is_static     : bool;
  method_is_final      : bool;
  method_is_abstract   : bool;
  method_name          : Litstr.id;
  method_params        : Hhas_param.t list;
  method_return_type   : Hhas_type_info.t option;
  method_body          : Hhbc_ast.instruct list;
}

let make
  method_attributes
  method_is_protected
  method_is_public
  method_is_private
  method_is_static
  method_is_final
  method_is_abstract
  method_name
  method_params
  method_return_type
  method_body = {
    method_attributes;
    method_is_protected;
    method_is_public;
    method_is_private;
    method_is_static;
    method_is_final;
    method_is_abstract;
    method_name;
    method_params;
    method_return_type;
    method_body
  }

let attributes method_def = method_def.method_attributes
let is_protected method_def = method_def.method_is_protected
let is_private method_def = method_def.method_is_private
let is_public method_def = method_def.method_is_public
let is_static method_def = method_def.method_is_static
let is_final method_def = method_def.method_is_final
let is_abstract method_def = method_def.method_is_abstract
let name method_def = method_def.method_name
let params method_def = method_def.method_params
let return_type method_def = method_def.method_return_type
let body method_def = method_def.method_body

let from_ast : A.class_ -> A.method_ -> t option =
  fun ast_class ast_method ->
  let class_tparams = tparams_to_strings ast_class.A.c_tparams in
  let method_name = Litstr.to_string @@ snd ast_method.A.m_name in
  let method_is_abstract = List.mem ast_method.A.m_kind A.Abstract in
  let method_is_final = List.mem ast_method.A.m_kind A.Final in
  let method_is_private = List.mem ast_method.A.m_kind A.Private in
  let method_is_protected = List.mem ast_method.A.m_kind A.Protected in
  let method_is_public = List.mem ast_method.A.m_kind A.Public in
  let method_is_static = List.mem ast_method.A.m_kind A.Static in
  let method_attributes =
    Emit_attribute.from_asts ast_method.A.m_user_attributes in
  match ast_method.A.m_body with
  | b ->
    let method_tparams = tparams_to_strings ast_method.A.m_tparams in
    let tparams = class_tparams @ method_tparams in
    let body_instrs, method_params, method_return_type =
      from_body tparams ast_method.A.m_params ast_method.A.m_ret b in
    let method_body = instr_seq_to_list body_instrs in
    let m = make
      method_attributes
      method_is_protected
      method_is_public
      method_is_private
      method_is_static
      method_is_final
      method_is_abstract
      method_name
      method_params
      method_return_type
      method_body in
    Some m

let from_asts ast_class ast_methods =
  List.filter_map ast_methods (from_ast ast_class)
