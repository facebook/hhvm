(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix

(** Return the type of the smallest expression node whose associated span
 * (the Pos.t in its Tast.ExprAnnotation.t) contains the given position.
 * "Smallest" here refers to the size of the node's associated span, in terms of
 * its byte length in the original source file.
 *
 * If there is no single smallest node (i.e., multiple expression nodes have
 * spans of the same length containing the given position, where that length is
 * less than the length of all other spans containing the given position),
 * return the type of the first of these nodes visited in a preorder traversal
 * of the Tast.
 *
 * This choice is somewhat arbitrary, and would seem to be unnecessary at first
 * blush (and indeed would be in a concrete syntax tree). In most situations,
 * each expression should have a distinct span, but consider a sugar
 * pseudofunction `log_arraykey` which is desugared as follows:
 *
 * log_arraykey($ak);
 * // desugars to:
 * if (is_int($ak)) { log_int($ak); }
 * if (is_string($ak)) { log_string($ak); }
 *
 * In this situation, four expressions in the TAST have an equivalent span
 * referring to the span of `$ak` in the original source. We return the type of
 * the first visited in a preorder traversal, the argument to `is_int`. This
 * expression will have the expected type (i.e., the type of `$ak` before it is
 * refined by `is_int` or `is_string` in desugared code).
 *
 * Multiple expressions could also be associated with the same span if we
 * introduced a purely-logical expression to the TAST, which had no syntactical
 * representation (i.e., it contained a single child: another expression with
 * the same associated span).
 *
 * The choice of returning the "smallest" expression is as a proxy for concrete
 * syntax specificity, where a child node (in the concrete syntax tree) is
 * considered more specific than its parent. We would like to return the type of
 * the most specific expression node containing the given position, but we
 * cannot assume that the structure of the CST is reflected in the TAST.
 *)

let base_visitor line char =
  object (self)
    inherit [_] Tast_visitor.reduce as super

    inherit [Pos.t * _ * _] Ast_defs.option_monoid

    method private merge lhs rhs =
      (* A node with position P is not always a parent of every other node with
       * a position contained by P. Some desugaring can cause nodes to be
       * rearranged so that this is no longer the case (e.g., `invariant`).
       *
       * To deal with this, we simply take the smaller node. *)
      let (lpos, _, _) = lhs in
      let (rpos, _, _) = rhs in
      if Pos.length lpos <= Pos.length rpos then
        lhs
      else
        rhs

    method! on_'ex env (pos, ty) =
      if Pos.inside pos line char then
        Some (pos, env, ty)
      else
        None

    method! on_xhp_simple env attribute =
      let (pos, _) = attribute.Aast.xs_name in
      if Pos.inside pos line char then
        Some (pos, env, attribute.Aast.xs_type)
      else
        super#on_xhp_simple env attribute

    method! on_class_id env cid =
      match cid with
      (* Don't use the resolved class type (the expr_annotation on the class_id
       type) when hovering over a CIexpr--we will want to show the type the
       expression is annotated with (e.g., classname<C>) and it will not have a
       smaller position. *)
      | (_, Aast.CIexpr e) -> self#on_expr env e
      | _ -> super#on_class_id env cid
  end

(** Return the type of the node associated with exactly the given range.

    When more than one node has the given range, return the type of the first
    node visited in a preorder traversal.
*)
let range_visitor startl startc endl endc =
  object
    inherit [_] Tast_visitor.reduce

    inherit [_] Ast_defs.option_monoid

    method merge x _ = x

    method! on_'ex env (pos, ty) =
      if
        Pos.exactly_matches_range
          pos
          ~start_line:startl
          ~start_col:startc
          ~end_line:endl
          ~end_col:endc
      then
        Some (env, ty)
      else
        None
  end

let type_at_pos
    (ctx : Provider_context.t) (tast : Tast.program) (line : int) (char : int) :
    (Tast_env.env * Tast.ty) option =
  (base_visitor line char)#go ctx tast >>| fun (_, env, ty) -> (env, ty)

let expanded_type_at_pos
    (ctx : Provider_context.t) (tast : Tast.program) (line : int) (char : int) :
    (Tast_env.env * Tast.ty) option =
  type_at_pos ctx tast line char
  |> Option.map ~f:(fun (env, ty) -> (env, Tast_expand.expand_ty env ty))

let type_at_range
    (ctx : Provider_context.t)
    (tast : Tast.program)
    (start_line : int)
    (start_char : int)
    (end_line : int)
    (end_char : int) : (Tast_env.env * Tast.ty) option =
  (range_visitor start_line start_char end_line end_char)#go ctx tast

let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : (string * string) option =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  type_at_pos ctx tast line column >>| fun (env, ty) ->
  ( Tast_env.print_ty env ty,
    Tast_env.ty_to_json env ty |> Hh_json.json_to_string )
