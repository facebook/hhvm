(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module CoroutineMethodLowerer = Coroutine_method_lowerer
module CoroutineClosureGenerator = Coroutine_closure_generator
module CoroutineStateMachineGenerator = Coroutine_state_machine_generator
module CoroutineSyntax = Coroutine_syntax
module EditableSyntax = Full_fidelity_editable_syntax
module EditableToken = Full_fidelity_editable_token
module List = Core_list
module Rewriter = Full_fidelity_rewriter.WithSyntax(EditableSyntax)
module SourceText = Full_fidelity_source_text
module SyntaxTree = Full_fidelity_syntax_tree

open CoroutineSyntax
open EditableSyntax
open Coroutine_type_lowerer

let option_flat_map o ~f =
  match Option.map o ~f with
  | Some ((Some _) as value) -> value
  | _ -> None

let maybe_get_token_text node =
  match syntax node with
  | Token token -> Some (EditableToken.text token)
  | _ -> None

(**
 * Returns the transformed method node and the generated closure's syntax.
 *)
let maybe_generate_methods_and_closure
    classish_name
    method_node
    header_node
    function_name =
  let state_machine_syntax =
    CoroutineStateMachineGenerator.generate_coroutine_state_machine
      classish_name
      function_name
      method_node
      header_node in
  let closure_syntax =
    CoroutineClosureGenerator.generate_coroutine_closure
      classish_name
      function_name
      method_node
      header_node in
  Option.map
    (CoroutineMethodLowerer.maybe_rewrite_methodish_declaration
      classish_name
      function_name
      method_node
      header_node)
    (fun rewritten_method_syntax ->
      [ rewritten_method_syntax; state_machine_syntax ], closure_syntax)

(* A void function becomes a unit function; if the annotation is missing it
becomes mixed. *)
let fix_up_header_node ({ function_colon; function_type; _; } as node) =
  let function_type = rewrite_return_type function_type in
  let function_colon =
    if is_missing function_colon then colon_syntax else function_colon in
  { node with function_type; function_colon }

(**
 * If the provided function header is for a coroutine, rewrites the declaration
 * header and the function body into a desugared coroutine implementation.
 * Also extracts the coroutine's closure.
 *)
let maybe_generate_methods_and_closure_from_header
    classish_name
    ({ methodish_function_decl_header; _; } as method_node) =
  match syntax methodish_function_decl_header with
  | FunctionDeclarationHeader
    ({ function_coroutine; function_name; _; } as header_node)
    when not (is_missing function_coroutine) ->
      let header_node = fix_up_header_node header_node in
      option_flat_map
        (maybe_get_token_text function_name)
        ~f:
          (maybe_generate_methods_and_closure
            classish_name
            method_node
            header_node)

  | _ ->
      (* Unexpected or malformed input, so we won't transform the coroutine. *)
      None

(**
 * If the provided methodish declaration is for a coroutine, generates the
 * appropriate methods and state machine for the coroutine.
 *)
let maybe_generate_methods_and_closure_from_method classish_name node =
  match syntax node with
  | MethodishDeclaration node ->
      maybe_generate_methods_and_closure_from_header
        classish_name
        node
  | _ ->
      (* Irrelevant input. *)
      None

(**
 * Accumulates node transforms.
 *
 * Transforms method nodes, accumulating all nodes. Accumulates whether at least
 * one node has been transformed.
 *)
let rewrite_classish_body_element
    classish_name
    (classish_body_elements_acc, closures_acc, any_rewritten_acc)
    classish_body_element_node =
  Option.value_map
    (maybe_generate_methods_and_closure_from_method
      classish_name
      classish_body_element_node)
    ~default:
      (classish_body_element_node :: classish_body_elements_acc,
        closures_acc,
        any_rewritten_acc)
    ~f:(fun (method_nodes, closure_node) ->
      method_nodes @ classish_body_elements_acc,
      closure_node :: closures_acc,
      true)

(**
 * Rewrites classish body elements. If at least one element is modified, then
 * returns Some with all of the nodes. Otherwise, returns None.
 *)
let maybe_rewrite_classish_body_elements
    classish_name classish_body_elemenets_node =
  match syntax classish_body_elemenets_node with
  | SyntaxList syntax_list ->
      let rewritten_nodes, closure_nodes, any_rewritten =
        List.fold
          ~f:(rewrite_classish_body_element classish_name)
          ~init:([], [], false)
          syntax_list in
      if any_rewritten then
        let rewritten_nodes = List.rev rewritten_nodes in
        let closure_nodes = List.rev closure_nodes in
        Some (make_list rewritten_nodes, closure_nodes)
      else
        None
  | _ ->
      (* Missing, unexpected, or malformed input, so we won't transform the
         class. *)
      None

(**
 * Rewrites the elements of the body.
 *)
let maybe_rewrite_classish_body classish_name classish_body_node =
  let make_syntax classish_body_node =
    make_syntax (ClassishBody classish_body_node) in
  match syntax classish_body_node with
  | ClassishBody ({ classish_body_elements; _; } as classish_body_node) ->
      Option.map
        (maybe_rewrite_classish_body_elements
          classish_name
          classish_body_elements)
        (fun (classish_body_elements, closure_nodes) ->
          make_syntax { classish_body_node with classish_body_elements; },
          closure_nodes)
  | _ ->
      (* Unexpected or malformed input, so we won't transform the coroutine. *)
      None

(**
 * If the class contains at least one coroutine method, then those methods are
 * rewritten, and closures are generated as necessary. Otherwise, the
 * class is not transformed.
 *)
let maybe_rewrite_class node =
  let make_syntax node = make_syntax (ClassishDeclaration node) in
  match syntax node with
  | ClassishDeclaration ({ classish_body; classish_name; _; } as node) ->
      (match maybe_get_token_text classish_name with
      | Some classish_name ->
          Option.map
            (maybe_rewrite_classish_body classish_name classish_body)
            ~f:(fun (classish_body, closure_nodes) ->
              make_syntax { node with classish_body; },
              closure_nodes)
      | _ ->
        (* Malformed class name. *)
        None)
  | _ ->
      (* Irrelevant input. *)
      None

(**
 * Rewrites toplevel classes. The class nodes themselves are transformed
 * directly. Generated closure nodes are collected and written alongside
 * of the class nodes.
 *)
let rewrite_classes (node_acc, any_rewritten_acc) node =
  Option.value_map
    (maybe_rewrite_class node)
    ~default:(node :: node_acc, any_rewritten_acc)
    ~f:(fun (node, closure_nodes) ->
      node :: (closure_nodes @ node_acc), true)

(**
 * Rewrites classes containing coroutine methods. Additional closure classes
 * may be generated alongside of the original class.
 *)
let maybe_rewrite_syntax_list node =
  match syntax node with
  | SyntaxList syntax_list ->
      let rewritten_nodes, any_rewritten =
        List.fold
          ~f:rewrite_classes
          ~init:([], false)
          syntax_list in
      if any_rewritten then Some (make_list rewritten_nodes) else None
  | _ ->
      (* Irrelevant input. *)
      None

(**
 * Bridge function so that maybe_rewrite_class returns a value compatible with
 * the Rewriter.
 *)
let rewrite node =
  Some (
    Option.value_map
      (maybe_rewrite_syntax_list node)
      ~default:(node, false)
      ~f:(fun node -> node, true)
  )

let lower_coroutines syntax_tree =
  syntax_tree
    |> from_tree
    |> Rewriter.rewrite_pre rewrite
    |> fst
    |> text
    |> SourceText.make
    |> SyntaxTree.make
