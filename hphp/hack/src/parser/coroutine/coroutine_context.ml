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
a coroutine M<T> inside class C<U> that returns int then we will need to
generate

class C_M_GeneratedClosure<T, U> extends ClosureBase<int>

Other information, like what variables need to be captured by the closure,
is computed elsewhere.
*)

module EditableSyntax = Full_fidelity_editable_syntax

type t = {
  classish_name : EditableSyntax.t;
  classish_type_parameters : EditableSyntax.t;
  function_name : EditableSyntax.t;
  function_type_parameter_list : EditableSyntax.t;
  function_type : EditableSyntax.t;
  (* Note that there is never a name conflict because in Hack,
  class C<T> { public function M<T>(){} }
  is illegal. *)
  (* TODO: Fix naming in parse tree schema; why is it "class type parameters"
  but "function type parameter list"? *)
}

let empty =
  let m = EditableSyntax.make_missing() in
  { classish_name = m;
    classish_type_parameters = m;
    function_name = m;
    function_type_parameter_list = m;
    function_type = m;
  }

let make_from_context parents =
  let rec aux parents acc =
    match parents with
    | [] -> acc
    | h :: t ->
      begin match EditableSyntax.syntax h with
      | EditableSyntax.ClassishDeclaration {
          EditableSyntax.classish_name;
          EditableSyntax.classish_type_parameters; _; } ->
        aux t { acc with classish_name; classish_type_parameters; }
      | EditableSyntax.MethodishDeclaration {
        EditableSyntax.methodish_function_decl_header = {
          EditableSyntax.syntax = EditableSyntax.FunctionDeclarationHeader {
            EditableSyntax.function_name;
            EditableSyntax.function_type_parameter_list;
            EditableSyntax.function_type; _; };
          _; }; _; } ->
        aux t { acc with
          function_name; function_type_parameter_list; function_type; }
      | EditableSyntax.FunctionDeclaration {
        EditableSyntax.function_declaration_header = {
          EditableSyntax.syntax = EditableSyntax.FunctionDeclarationHeader {
            EditableSyntax.function_name;
            EditableSyntax.function_type_parameter_list;
            EditableSyntax.function_type; _; };
          _; }; _; } ->
        aux t { acc with
          function_name; function_type_parameter_list; function_type }
      | _ -> aux t acc
      end
  in
  aux parents empty
