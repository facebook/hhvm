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
let rewrite_coroutine_annotation
    ({
      closure_parameter_types;
      closure_return_type;
      _;
    } as original_type) =
  let new_return_type =
    CoroutineTypeLowerer.rewrite_return_type closure_return_type in
  let continuation_type = make_continuation_type_syntax new_return_type in
  let new_parameter_types = prepend_to_comma_delimited_syntax_list
    continuation_type closure_parameter_types in
  let coroutine_return_type =
    make_coroutine_result_type_syntax new_return_type in
  make_syntax (
    ClosureTypeSpecifier {
      original_type with
        closure_coroutine = make_missing ();
        closure_parameter_types = new_parameter_types;
        closure_return_type = coroutine_return_type;
    }
  )

(* TODO: Rename anonymous_parameters / function_parameter_list to match. *)
let lower_coroutine_anon
    context
    ({ anonymous_body; anonymous_parameters; _; } as anon) =
  let ({anonymous_type; _;} as anon) =
    rewrite_anon_function_return_type anon in
  let anonymous_body, closure_syntax =
    CoroutineStateMachineGenerator.generate_coroutine_state_machine
      context
      anonymous_body
      anonymous_type
      anonymous_parameters in
  let anon = { anon with anonymous_body } in
  let anon = CoroutineMethodLowerer.rewrite_anon context anon in
  (anon, closure_syntax)

let lower_coroutine_lambda
    context
    ({ lambda_parameters; _; } as lambda_signature)
    lambda_body
    lambda =
  let ({lambda_type; _;} as lambda_signature) =
    rewrite_lambda_return_type lambda_signature in
  let lambda_body, closure_syntax =
    CoroutineStateMachineGenerator.generate_coroutine_state_machine
      context
      lambda_body
      lambda_type
      lambda_parameters in
  let lambda = { lambda with lambda_body } in
  let lambda = CoroutineMethodLowerer.rewrite_lambda
    context lambda_signature lambda in
  (lambda, closure_syntax)

let rewrite_method_or_function
    context
    ({function_parameter_list; _;} as original_header_node)
    original_body =
  let ({function_type; _;} as new_header_node) =
    rewrite_function_header_return_type original_header_node in
  let new_body, closure_syntax =
    CoroutineStateMachineGenerator.generate_coroutine_state_machine
      context
      original_body
      function_type
      function_parameter_list in
  (new_header_node, new_body, closure_syntax)

let lower_coroutine_function
    context
    original_header
    original_body
    original_function =
  let (new_header_node, new_body, closure_syntax) = rewrite_method_or_function
    context original_header original_body in
  let new_function_syntax =
    CoroutineMethodLowerer.rewrite_function_declaration
      context
      original_function
      new_header_node
      new_body in
  (closure_syntax, new_function_syntax)

let fix_up_lambda_body lambda_body =
  match syntax lambda_body with
  | CompoundStatement _ -> lambda_body
  | _ ->
    let stmt = make_return_statement_syntax lambda_body in
    make_compound_statement_syntax [stmt]

let lower_coroutine_functions_and_types
    parents
    current_node
    ((closures, lambda_count) as current_acc) =
  match syntax current_node with
  | FunctionDeclaration ({
      function_declaration_header = {
        syntax = FunctionDeclarationHeader ({
          function_coroutine; _;
        } as header_node); _;
      };
      function_body; _;
    } as function_node) when not @@ is_missing function_coroutine ->
      let context = Coroutine_context.make_from_context
        current_node parents None in
      let (closure_syntax, new_function_syntax) = lower_coroutine_function
        context header_node function_body function_node in
      ((closure_syntax :: closures, lambda_count),
        Rewriter.Result.Replace new_function_syntax)
  | LambdaExpression ({
    lambda_coroutine;
    lambda_signature = { syntax = LambdaSignature lambda_signature; _; };
    lambda_body;
    _;
    } as lambda) when not @@ is_missing lambda_coroutine ->
    let context = Coroutine_context.make_from_context
      current_node parents (Some lambda_count) in
    let lambda_body = fix_up_lambda_body lambda_body in
    let (lambda, closure_syntax) =
      lower_coroutine_lambda context lambda_signature lambda_body lambda in
    ((closure_syntax :: closures, (lambda_count + 1)),
      Rewriter.Result.Replace lambda)
  | AnonymousFunction ({
    anonymous_coroutine_keyword;
    _;
    } as anon) when not @@ is_missing anonymous_coroutine_keyword ->
      let context = Coroutine_context.make_from_context
        current_node parents (Some lambda_count) in
      let (anon, closure_syntax) = lower_coroutine_anon context anon in
      ((closure_syntax :: closures, (lambda_count + 1)),
        Rewriter.Result.Replace anon)
  | MethodishDeclaration ({
      methodish_function_decl_header = {
        syntax = FunctionDeclarationHeader ({
          function_coroutine; _;
        } as header_node); _;
      };
      methodish_function_body; _;
    } as method_node) when not @@ is_missing function_coroutine ->
    let context = Coroutine_context.make_from_context
      current_node parents None in
    let (new_header_node, new_body, closure_syntax) =
      rewrite_method_or_function
        context
        header_node
        methodish_function_body in
    let new_method_syntax =
      CoroutineMethodLowerer.rewrite_methodish_declaration
        context
        method_node
        new_header_node
        new_body in
    ((closure_syntax :: closures, lambda_count),
      Rewriter.Result.Replace new_method_syntax)
  | ClosureTypeSpecifier ({ closure_coroutine; _; } as type_node)
    when not @@ is_missing closure_coroutine ->
      let new_type_node = rewrite_coroutine_annotation type_node in
      (current_acc, Rewriter.Result.Replace new_type_node)
  | _ ->
    (current_acc, Rewriter.Result.Keep)

(*
We are working around a significant shortcoming of HHVM here.  We are supposed
to have an invariant that the order in which type declarations appear in a
Hack file is irrelevant, but this is not the case:

interface I {}
class B implements I {}
new D(); // Crashes here at runtime
class D extends B {}

The crash is due to a peculiarity in how HHVM handles interfaces.

The closure classes extend the closure base, which implements an interface.
We can therefore very easily get into this situation when generating closure
classes at the end of a file.

What we do then is gather up *all* the classes in a file, sort them to the
top of the file, follow them with the closure classes, and then the rest
of the code in the file.

This unfortunate code can be removed when the bug is fixed in HHVM, and
we can simply append the closure classes to the end of the list of
declarations.
*)
let rewrite_script closures root =
  match closures with
  | [] -> root
  | _ ->
    begin match syntax root with
    | Script { script_declarations } ->
      let script_declarations = syntax_node_to_list script_declarations in
      let (types, not_types) =
        List.partition_tf script_declarations ~f:is_classish_declaration in
      begin match not_types with
      | h :: t -> make_script (make_list (h :: (types @ closures @ t)))
      | [] -> failwith "How did we get a script with no header element?"
      end
    | _ -> failwith "How did we get a root that is not a script?"
    end

let lower_coroutines syntax_tree =
  let root = from_tree syntax_tree in
  let ((closures, _), root) = Rewriter.parented_aggregating_rewrite_post
    lower_coroutine_functions_and_types root ([], 0) in
  root
    |> rewrite_script (List.rev closures)
    |> text
    |> SourceText.make
    |> SyntaxTree.make
