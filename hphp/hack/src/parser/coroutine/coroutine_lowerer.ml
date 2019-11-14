(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Core_kernel
module CoroutineMethodLowerer = Coroutine_method_lowerer
module CoroutineStateMachineGenerator = Coroutine_state_machine_generator
module CoroutineSyntax = Coroutine_syntax
module CoroutineTypeLowerer = Coroutine_type_lowerer
module CoroutineSuspendRewriter = Coroutine_suspend_rewriter
module Syntax = Full_fidelity_editable_positioned_syntax
module Rewriter = Full_fidelity_rewriter.WithSyntax (Syntax)
open Syntax
open CoroutineSyntax
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
    ({ closure_parameter_list; closure_return_type; _ } as original_type) =
  let new_return_type =
    CoroutineTypeLowerer.rewrite_return_type closure_return_type
  in
  let continuation_parameter =
    make_continuation_closure_parameter_syntax new_return_type
  in
  let new_parameter_list =
    prepend_to_comma_delimited_syntax_list
      continuation_parameter
      closure_parameter_list
  in
  let coroutine_return_type =
    make_coroutine_result_type_syntax new_return_type
  in
  make_syntax
    (from_closure_type_specifier
       {
         original_type with
         closure_coroutine = make_missing ();
         closure_parameter_list = new_parameter_list;
         closure_return_type = coroutine_return_type;
       })

let lower_coroutine_anon context anon_node =
  let ({ anonymous_body; _ } as anon) =
    get_anonymous_function (syntax anon_node)
  in
  let ({ anonymous_type; _ } as anon) =
    rewrite_anon_function_return_type anon
  in
  let (anonymous_body, closure_syntax) =
    CoroutineStateMachineGenerator.generate_coroutine_state_machine
      context
      anonymous_body
      anonymous_type
  in
  let anon = { anon with anonymous_body } in
  let anon = Syntax.synthesize_from anon_node (from_anonymous_function anon) in
  let anon = CoroutineMethodLowerer.rewrite_anon context anon in
  (anon, closure_syntax)

let lower_coroutine_lambda context lambda_signature lambda_body lambda_node =
  let lambda = get_lambda_expression (syntax lambda_node) in
  let ({ lambda_type; _ } as lambda_signature) =
    rewrite_lambda_return_type lambda_signature
  in
  let (lambda_body, closure_syntax) =
    CoroutineStateMachineGenerator.generate_coroutine_state_machine
      context
      lambda_body
      lambda_type
  in
  let lambda = { lambda with lambda_body } in
  let lambda =
    Syntax.synthesize_from lambda_node (from_lambda_expression lambda)
  in
  let lambda =
    CoroutineMethodLowerer.rewrite_lambda context lambda_signature lambda
  in
  (lambda, closure_syntax)

let rewrite_method_or_function context original_header_node original_body =
  let ({ function_type; _ } as new_header_node) =
    rewrite_function_header_return_type original_header_node
  in
  let (new_body, closure_syntax) =
    CoroutineStateMachineGenerator.generate_coroutine_state_machine
      context
      original_body
      function_type
  in
  (new_header_node, new_body, closure_syntax)

let lower_coroutine_function context original_header original_body =
  let (new_header_node, new_body, closure_syntax) =
    rewrite_method_or_function context original_header original_body
  in
  let new_function_syntax =
    CoroutineMethodLowerer.rewrite_function_declaration
      context
      new_header_node
      new_body
  in
  (closure_syntax, new_function_syntax)

let lower_coroutine_functions_and_types
    parents current_node ((closures, lambda_count) as current_acc) =
  match syntax current_node with
  | FunctionDeclaration
      {
        function_declaration_header =
          {
            syntax =
              FunctionDeclarationHeader { function_modifiers = m; _ } as
              header_node;
            _;
          };
        function_body;
        _;
      }
    when has_coroutine_modifier m ->
    let context =
      Coroutine_context.make_from_context current_node parents None
    in
    let (closure_syntax, new_function_syntax) =
      lower_coroutine_function
        context
        (get_function_declaration_header header_node)
        function_body
    in
    ( (Option.to_list closure_syntax @ closures, lambda_count),
      Rewriter.Result.Replace new_function_syntax )
  | LambdaExpression
      {
        lambda_coroutine;
        lambda_signature = { syntax = LambdaSignature _ as lambda_signature; _ };
        lambda_body;
        _;
      }
    when not @@ is_missing lambda_coroutine ->
    let context =
      Coroutine_context.make_from_context
        current_node
        parents
        (Some lambda_count)
    in
    let lambda_body = CoroutineSuspendRewriter.fix_up_lambda_body lambda_body in
    let (lambda, closure_syntax) =
      lower_coroutine_lambda
        context
        (get_lambda_signature lambda_signature)
        lambda_body
        current_node
    in
    ( (Option.to_list closure_syntax @ closures, lambda_count + 1),
      Rewriter.Result.Replace lambda )
  | AnonymousFunction { anonymous_coroutine_keyword; _ }
    when not @@ is_missing anonymous_coroutine_keyword ->
    let context =
      Coroutine_context.make_from_context
        current_node
        parents
        (Some lambda_count)
    in
    let (anon, closure_syntax) = lower_coroutine_anon context current_node in
    ( (Option.to_list closure_syntax @ closures, lambda_count + 1),
      Rewriter.Result.Replace anon )
  | MethodishDeclaration
      {
        methodish_function_decl_header =
          {
            syntax =
              FunctionDeclarationHeader { function_modifiers = m; _ } as
              header_node;
            _;
          };
        methodish_function_body;
        _;
      }
    when has_coroutine_modifier m ->
    let context =
      Coroutine_context.make_from_context current_node parents None
    in
    let (new_header_node, new_body, closure_syntax) =
      rewrite_method_or_function
        context
        (get_function_declaration_header header_node)
        methodish_function_body
    in
    let new_method_syntax =
      CoroutineMethodLowerer.rewrite_methodish_declaration
        context
        new_header_node
        new_body
    in
    ( (Option.to_list closure_syntax @ closures, lambda_count),
      Rewriter.Result.Replace new_method_syntax )
  | ClosureTypeSpecifier { closure_coroutine; _ } as type_node
    when not @@ is_missing closure_coroutine ->
    let new_type_node =
      rewrite_coroutine_annotation (get_closure_type_specifier type_node)
    in
    (current_acc, Rewriter.Result.Replace new_type_node)
  | _ -> (current_acc, Rewriter.Result.Keep)

(**
 * Appends the rewritten declaration onto the list of closures that were
 * generated when rewritting that declaration
 *)
let combine_declaration closures declaration =
  if is_classish_declaration declaration then
    declaration :: closures
  else
    closures @ [declaration]

(**
 * Namespace declarations are a little harder to rewrite because we need to
 * ensure that any closures that are generated from code within the namespace
 * remain in the namespace. Additionally, since use statements can be used
 * within a namespace body, it is necessary to partition the declarations in
 * the namespace body
 *)
let rec rewrite_namespace_declaration node lambda_count =
  match syntax node with
  | NamespaceDeclaration
      ( {
          namespace_body =
            {
              syntax =
                NamespaceBody ({ namespace_declarations; _ } as namespace_body_s);
              _;
            } as namespace_body;
          _;
        } as namespace_declaration_s ) ->
    let namespace_declaration_list =
      syntax_node_to_list namespace_declarations
    in
    let (lambda_count, namespace_declarations) =
      rewrite_declaration_acc lambda_count namespace_declaration_list
    in
    let namespace_declarations = make_list namespace_declarations in
    let namespace_body =
      Syntax.synthesize_from
        namespace_body
        (NamespaceBody { namespace_body_s with namespace_declarations })
    in
    let new_declaration =
      Syntax.synthesize_from
        node
        (NamespaceDeclaration { namespace_declaration_s with namespace_body })
    in
    (([], lambda_count), new_declaration)
  | _ -> (([], lambda_count), node)

(**
 * Rewrites a top level declaration, appends the closures generated
 * and then appends the result onto the accumulating list of rewritten
 * declarations
 *)
and rewrite_declaration node (lambda_count, previous_declarations) =
  let ((closures, lambda_count), rewritten_node) =
    if is_namespace_declaration node then
      rewrite_namespace_declaration node lambda_count
    else
      Rewriter.parented_aggregating_rewrite_post
        lower_coroutine_functions_and_types
        node
        ([], lambda_count)
  in
  let closures = List.rev closures in
  let rewritten_declaration = combine_declaration closures rewritten_node in
  (lambda_count, rewritten_declaration @ previous_declarations)

(**
 * Rewrites a list of declarations, taking in a lambda count accumulator
 *)
and rewrite_declaration_acc lambda_count declaration =
  List.fold_right ~f:rewrite_declaration ~init:(lambda_count, []) declaration

let rewrite_all_declarations declaration_list =
  let (_, rewritten_declarations) =
    rewrite_declaration_acc 0 declaration_list
  in
  rewritten_declarations

(**
 * Because there are many instances in the lowered code where we are unable
 * to annotate types, files with // strict in them will not pass typechecking
 * when lowered, so we need to strip the strict mode when lowering the files
 *
 * We only strip the trailing text if the trailing text doesn't match // decl
 *)
let remove_strict_mode hh_decl =
  let remove_strict_trailing_text node =
    match syntax node with
    | MarkupSuffix
        ( { markup_suffix_name = { syntax = Token name; _ } as suffix_name; _ }
        as markup_suffix ) ->
      if matches_decl name then
        Rewriter.Result.Keep
      else
        let new_suffix = Token.with_trailing_text " // partial\n" name in
        let markup_suffix_name =
          Syntax.synthesize_from suffix_name (Token new_suffix)
        in
        let new_node =
          Syntax.synthesize_from
            node
            (MarkupSuffix { markup_suffix with markup_suffix_name })
        in
        Rewriter.Result.Replace new_node
    | _ -> Rewriter.Result.Keep
  in
  Rewriter.rewrite_post remove_strict_trailing_text hh_decl

(**
 Lowers all coroutines found in a script

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
let lower_coroutines root =
  match syntax root with
  | Script { script_declarations } ->
    let declarations = syntax_node_to_list script_declarations in
    begin
      match declarations with
      | hh_decl :: declarations ->
        let hh_decl = remove_strict_mode hh_decl in
        let rewritten_declarations = rewrite_all_declarations declarations in
        let rewritten_declarations = hh_decl :: rewritten_declarations in
        make_script (make_list rewritten_declarations)
      | _ -> failwith "How did we get a script with no header element?"
    end
  | _ -> failwith "How did we get a root that is not a script?"
