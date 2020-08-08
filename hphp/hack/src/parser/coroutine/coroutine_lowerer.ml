(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude
module CoroutineSyntax = Coroutine_syntax
module Syntax = Full_fidelity_editable_positioned_syntax
module Rewriter = Full_fidelity_rewriter.WithSyntax (Syntax)
open Syntax
open CoroutineSyntax
open Coroutine_type_lowerer

let lower_coroutine_functions_and_types
    _parents _current_node ((_closures, _lambda_count) as current_acc) =
  (current_acc, Rewriter.Result.Keep)

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
