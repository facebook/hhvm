(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Linting_visitors

let untyped_linters =
  [
    Linter_clone.go;
    Linter_foreach_shadow.go;
    Linter_invariant_violation.go;
    Linter_await_in_loop.go;
  ]
  @ Linting_service.untyped_linters

let typed_linters =
  [
    Linter_equality_check.handler;
    Linter_disjoint_types.handler;
    Linter_is_checks.handler;
    Linter_switch_check.handler;
    Linter_missing_override_attribute.handler;
    Linter_sketchy_null_check.handler;
    Linter_truthiness_test.handler;
    Linter_redundant_generics.handler;
    Linter_class_overrides_trait.handler;
    Linter_expr_tree_types.handler;
    Linter_nullsafe_not_needed.handler;
    Linter_duplicate_properties.handler;
    Linter_loose_unsafe_cast.handler;
    Linter_redundant_cast.handler;
    Linter_xhp_attr_value.handler;
    Linter_pointless_booleans.handler;
    Linter_comparing_booleans.handler;
    Linter_unconditional_recursion.handler;
    Linter_branches_return_same_value.handler;
    Linter_internal_class.handler;
    Linter_async_lambda.handler;
    Linter_cast_non_primitive.handler;
  ]
  @ Linting_service.typed_linters

let lint_tast ctx (tast : Tast.program) =
  (Tast_visitor.iter_with typed_linters)#go ctx tast;
  Linting_service.lint_tast ctx tast

(* Most lint rules are easier to write against the named AST. However, some
 * things that we want to lint for are discarded / simplified by the time we
 * reach that stage, so we have a few lint rules that get checked during or
 * right after parsing. *)
let parse_and_lint fn content ctx =
  let parser_result =
    Errors.ignore_ (fun () ->
        Full_fidelity_ast.defensive_program
          ~elaborate_namespaces:true
          (Provider_context.get_tcopt ctx)
          fn
          content)
  in

  let { Parser_return.ast; _ } = parser_result in
  Linter_ast.go ast;
  parser_result

let lint_nast tcopt fn pr =
  Linting_visitors.reset ();
  List.iter (fun go -> go tcopt fn pr) untyped_linters;

  (* Run Nast visitors *)
  body_visitor_invoker#on_file () tcopt fn pr;
  ()

let lint ctx fn content =
  let orig_trace = !Typing_deps.trace in
  Typing_deps.trace := false;
  Errors.ignore_ (fun () ->
      let parser_return = parse_and_lint fn content ctx in
      let { Parser_return.file_mode; ast = full_ast; _ } = parser_return in
      (* naming and typing currently don't produce any lint errors *)
      (* PHP files generate declarations via some fairly error-prone regexps,
       * so only try to lint Hack files *)
      match file_mode with
      | None -> ()
      | Some _ ->
        (* Note that the named ASTs that we lint here aren't generated from the
         * unnamed ASTs that parse_and_lint generated; we're using the ones
         * already in the naming heap, which hh_server keeps up-to-date based
         * on inotify events. This is a little ugly, and might be worth
         * cleaning up at some point. *)
        lint_nast ctx fn parser_return;

        (* Get Typed AST and run TAST linters *)
        let (_, tast) =
          Typing_check_job.calc_errors_and_tast ctx fn ~full_ast
        in
        (* collect both the dynamic TAST and non-dynamic TAST in a big list,
           that's what linting expects! *)
        let tasts =
          Tast.tasts_as_list tast
          |> Tast_with_dynamic.collect
          |> Tast_with_dynamic.all
          |> List.concat
        in
        lint_tast ctx tasts);
  Typing_deps.trace := orig_trace
