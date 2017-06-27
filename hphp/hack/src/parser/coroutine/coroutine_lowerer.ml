(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module CoroutineMethodLowerer = Coroutine_method_lowerer
module CoroutineStateMachineGenerator = Coroutine_state_machine_generator
module CoroutineSyntax = Coroutine_syntax
module CoroutineTypeLowerer = Coroutine_type_lowerer
module EditableSyntax = Full_fidelity_editable_syntax
module EditableToken = Full_fidelity_editable_token
module List = Core_list
module Rewriter = Full_fidelity_rewriter.WithSyntax(EditableSyntax)
module SourceText = Full_fidelity_source_text
module SyntaxTree = Full_fidelity_syntax_tree

open CoroutineSyntax
open EditableSyntax
open Coroutine_type_lowerer

(**
 * Rewrites coroutine annotations.
 *
 * The following:
 *
 *   public function returnVoidVoidCoroutineLambda(
 *   ): (coroutine function(): void) { ... }
 *
 * Will be rewritten into:
 *
 *   public function returnVoidVoidCoroutineLambda(
 *   ): (function(
 *     CoroutineContinuation<CoroutineUnit>
 *   ): CoroutineResult<CoroutineUnit>) { ... }
 *
 * The following:
 *
 *   public function returnIntIntCoroutineLambda(
 *   ): (coroutine function(int): int) { ... }
 *
 * Will be rewritten into:
 *
 *    public function returnIntIntCoroutineLambda(
 *    ): (function(
 *      CoroutineContinuation<int>,
 *      int,
 *    ): CoroutineResult<int>) { ... }
 *)
let maybe_rewrite_coroutine_annotations node =
  match syntax node with
  | ClosureTypeSpecifier ({
      closure_coroutine = {
        syntax = Token { EditableToken.kind = TokenKind.Coroutine; _; };
        _;
      };
      closure_parameter_types;
      closure_return_type;
      _;
    } as node) ->
      Rewriter.Result.Replace (
        let closure_return_type =
          CoroutineTypeLowerer.rewrite_return_type closure_return_type in
        make_syntax (
          ClosureTypeSpecifier {
            node with
              closure_coroutine = make_missing ();
              closure_parameter_types =
                prepend_to_comma_delimited_syntax_list
                  (make_continuation_type_syntax closure_return_type)
                  closure_parameter_types;
              closure_return_type =
                make_coroutine_result_type_syntax closure_return_type;
          }
        )
      )
  | _ -> Rewriter.Result.Keep

(**
 * Returns the transformed method node and the generated closure's syntax.
 *)
let maybe_generate_methods_and_closure
    class_node
    method_node
    header_node =
  let rewritten_body, closure_syntax =
    CoroutineStateMachineGenerator.generate_coroutine_state_machine
      class_node
      method_node
      header_node in
  Option.map
    (CoroutineMethodLowerer.maybe_rewrite_methodish_declaration
      class_node
      method_node
      header_node
      rewritten_body)
    (* TODO: The rewritten method syntax is a singleton, not a list, and
    the set of closures generated could be a list, not a singleton. *)
    (fun rewritten_method_syntax ->
      [ rewritten_method_syntax ], closure_syntax)

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
    class_node
    ({ methodish_function_decl_header; _; } as method_node) =
  match syntax methodish_function_decl_header with
  | FunctionDeclarationHeader
    ({ function_coroutine; _; } as header_node)
    (* TODO: We need to rewrite non-coroutine functions if they contain
    coroutine lambdas. *)
    when not (is_missing function_coroutine) ->
      maybe_generate_methods_and_closure
        class_node
        method_node
        (fix_up_header_node header_node)
  | _ ->
      (* Unexpected or malformed input, so we won't transform the coroutine. *)
      None

(**
 * If the provided methodish declaration is for a coroutine, generates the
 * appropriate methods and state machine for the coroutine.
 *)
let maybe_generate_methods_and_closure_from_method class_node node =
  match syntax node with
  | MethodishDeclaration node ->
      maybe_generate_methods_and_closure_from_header class_node node
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
    class_node
    (classish_body_elements_acc, closures_acc, any_rewritten_acc)
    classish_body_element_node =
  Option.value_map
    (maybe_generate_methods_and_closure_from_method
      class_node
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
    class_node classish_body_elemenets_node =
  match syntax classish_body_elemenets_node with
  | SyntaxList syntax_list ->
      let rewritten_nodes, closure_nodes, any_rewritten =
        List.fold
          ~f:(rewrite_classish_body_element class_node)
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
let maybe_rewrite_classish_body ({ classish_body; _; } as class_node) =
  let make_syntax classish_body_node =
    make_syntax (ClassishBody classish_body_node) in
  match syntax classish_body with
  | ClassishBody ({ classish_body_elements; _; } as classish_body_node) ->
      Option.map
        (maybe_rewrite_classish_body_elements
          class_node
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
  (* TODO: We need to rewrite top-level coroutine methods *)
  (* TODO: Do we need to rewrite top-level statements that contain coroutine
  lambdas? *)
  let make_syntax node = make_syntax (ClassishDeclaration node) in
  match syntax node with
  | ClassishDeclaration class_node ->
      Option.map
        (maybe_rewrite_classish_body class_node)
        ~f:(fun (classish_body, closure_nodes) ->
          make_syntax { class_node with classish_body; },
          closure_nodes)
  | _ ->
      (* Irrelevant input. *)
      None

(**
 * Rewrites toplevel classes. The class nodes themselves are transformed
 * directly. Generated closure nodes are collected and written alongside
 * of the class nodes.
 *)
let rewrite_classes node (node_acc, any_rewritten_acc) =
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
        List.fold_right
          ~f:rewrite_classes
          ~init:([], false)
          syntax_list in
      if any_rewritten then
        Rewriter.Result.Replace (make_list rewritten_nodes)
      else
        Rewriter.Result.Keep
  | _ ->
      (* Irrelevant input. *)
      Rewriter.Result.Keep

let lower_coroutines syntax_tree =
  syntax_tree
    |> from_tree
    |> Rewriter.rewrite_post maybe_rewrite_coroutine_annotations
    |> Rewriter.rewrite_post maybe_rewrite_syntax_list
    |> text
    |> SourceText.make
    |> SyntaxTree.make
