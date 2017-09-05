(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

(* This type contains the contextual information we need to generate
the declaration of a coroutine's closure class. For example, if we have
a coroutine M<T> inside class C<U> then we will need to generate

class C_M_GeneratedClosure<T, U>

Other information, like what variables need to be captured by the closure,
what the return type and parameter types of the coroutine are, and so on,
is computed elsewhere.
*)

module Syntax = Full_fidelity_editable_positioned_syntax

type t = {
  classish_name : Syntax.t;
  classish_type_parameters : Syntax.t;
  function_name : Syntax.t;
  function_type_parameter_list : Syntax.t;
  original_node : Syntax.t;
  (* Note that there is never a name conflict because in Hack,
  class C<T> { public function M<T>(){} }
  is illegal. *)
  lambda_count : int option;
  parents : Syntax.t list;
  (* TODO: Fix naming in parse tree schema; why is it "class type parameters"
  but "function type parameter list"? *)
}

let empty =
  let m = Syntax.make_missing() in
  { classish_name = m;
    classish_type_parameters = m;
    function_name = m;
    function_type_parameter_list = m;
    original_node = m;
    lambda_count = None;
    parents = [];
  }

(* Note that the code below assumes that the parents list contains
no more than one named function and no more than one class. PHP supports
nested named functions; Hack does not. If Hack ever supports nested
classes or nested named functions then this code will have to change.
(Though of course if Hack supports nested classes then generating closures
for methods becomes easier!) *)
let make_from_context original_node parents lambda_count =
  let folder acc h =
    match Syntax.syntax h with
    | Syntax.ClassishDeclaration {
        Syntax.classish_name;
        Syntax.classish_type_parameters; _; } ->
      { acc with classish_name; classish_type_parameters; }
    | Syntax.MethodishDeclaration {
      Syntax.methodish_function_decl_header = {
        Syntax.syntax = Syntax.FunctionDeclarationHeader {
          Syntax.function_name;
          Syntax.function_type_parameter_list; _; };
        _; }; _; } ->
      { acc with
        function_name; function_type_parameter_list }
    | Syntax.FunctionDeclaration {
      Syntax.function_declaration_header = {
        Syntax.syntax = Syntax.FunctionDeclarationHeader {
          Syntax.function_name;
          Syntax.function_type_parameter_list; _; };
        _; }; _; } ->
      { acc with
        function_name; function_type_parameter_list }
    | _ -> acc
  in
  { (Core_list.fold ~f:folder ~init:empty (original_node :: parents))
    with lambda_count; parents; original_node }
