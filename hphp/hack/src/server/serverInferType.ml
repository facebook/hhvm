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

(* Is this expression indexing into a value of type shape?
 * E.g. $some_shope['foo'].
 *
 * If so, return the receiver.
 *)
let shape_indexing_receiver env (_, _, expr_) : Tast.expr option =
  match expr_ with
  | Aast.Array_get (((recv_ty, _, _) as recv), _) ->
    let ty = Tast_env.fully_expand env recv_ty in
    let (_, ty_) = Typing_defs_core.deref ty in
    (match ty_ with
    | Typing_defs_core.Tshape _ -> Some recv
    | _ -> None)
  | _ -> None

let class_const_ty env (cc : Tast.class_const) : Tast.ty option =
  let open Aast in
  match cc.cc_type with
  | Some hint ->
    let decl_ty = Tast_env.hint_to_ty env hint in
    let (_, ty) = Tast_env.localize_no_subst env ~ignore_errors:true decl_ty in
    Some ty
  | None ->
    let init_expr =
      match cc.cc_kind with
      | CCConcrete e -> Some e
      | CCAbstract e_opt -> e_opt
    in
    init_expr >>| fun (ty, _, _) -> ty

let class_id_ty env (id : Ast_defs.id) : Tast.ty =
  let (p, _) = id in
  let hint = (p, Aast.Happly (id, [])) in
  let decl_ty = Tast_env.hint_to_ty env hint in
  let (_, ty) = Tast_env.localize_no_subst env ~ignore_errors:true decl_ty in
  ty

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

let base_visitor ~human_friendly ~under_dynamic line_char_pairs =
  let size = List.length line_char_pairs in
  let zero = List.init size ~f:(fun _ -> None) in
  object (self)
    inherit [_] Tast_visitor.reduce as super

    inherit [(Pos.t * _ * _) option list] Visitors_runtime.monoid

    method private select_pos pos env ty =
      let correct_assumptions = self#correct_assumptions env in
      List.map line_char_pairs ~f:(fun (line, char) ->
          if Pos.inside pos line char && correct_assumptions then
            Some (pos, env, ty)
          else
            None)

    method private correct_assumptions env =
      let is_under_dynamic_assumptions =
        (Tast_env.tast_env_as_typing_env env).Typing_env_types.checked
        |> Tast.is_under_dynamic_assumptions
      in
      Bool.equal is_under_dynamic_assumptions under_dynamic

    method private zero = zero

    method private plus lhss rhss =
      let merge lhs rhs =
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
      in
      let merge_opt lhs rhs =
        match (lhs, rhs) with
        | (Some lhs, Some rhs) -> Some (merge lhs rhs)
        | (Some lhs, None) -> Some lhs
        | (None, Some rhs) -> Some rhs
        | (None, None) -> None
      in
      List.map2_exn lhss rhss ~f:merge_opt

    method! on_expr env ((ty, pos, _) as expr) =
      let res = self#select_pos pos env ty in
      match shape_indexing_receiver env expr with
      | Some recv when human_friendly ->
        (* If we're looking at a shape indexing expression, we don't
           want to recurse on the string literal.

           For example, if we have the code $user['age'] and hover
           over 'age', we want the hover type to be int, not string. *)
        self#plus res (self#on_expr env recv)
      | _ -> self#plus res (super#on_expr env expr)

    method! on_fun_param env fp =
      let res =
        self#select_pos fp.Aast.param_pos env fp.Aast.param_annotation
      in
      self#plus res (super#on_fun_param env fp)

    method! on_capture_lid env ((ty, (pos, _)) as cl) =
      let res = self#select_pos pos env ty in
      self#plus res (super#on_capture_lid env cl)

    method! on_xhp_simple env attribute =
      let (pos, _) = attribute.Aast.xs_name in
      let res = self#select_pos pos env attribute.Aast.xs_type in
      self#plus res (super#on_xhp_simple env attribute)

    method! on_class_id env ((ty, pos, _) as cid) =
      match cid with
      (* Don't use the resolved class type (the expr_annotation on the class_id
         type) when hovering over a CIexpr--we will want to show the type the
         expression is annotated with (e.g., classname<C>) and it will not have a
         smaller position. *)
      | (_, _, Aast.CIexpr e) -> self#on_expr env e
      | _ ->
        let res = self#select_pos pos env ty in
        self#plus res (super#on_class_id env cid)

    method! on_class_const env cc =
      let acc = super#on_class_const env cc in

      let (pos, _) = cc.Aast.cc_id in
      match class_const_ty env cc with
      | Some ty ->
        let res = self#select_pos pos env ty in
        self#plus res acc
      | None -> acc

    method! on_EnumClassLabel env id label_name =
      let acc = super#on_EnumClassLabel env id label_name in
      match id with
      | Some ((pos, _) as id) ->
        let ty = class_id_ty env id in
        let res = self#select_pos pos env ty in
        self#plus res acc
      | _ -> acc

    method! on_If env cond then_block else_block =
      match ServerUtils.resugar_invariant_call env cond then_block with
      | Some e -> self#on_expr env e
      | None -> super#on_If env cond then_block else_block
  end

(** Return the type of the node associated with exactly the given range.

    When more than one node has the given range, return the type of the first
    node visited in a preorder traversal.
*)
let range_visitor startl startc endl endc =
  object
    inherit [_] Tast_visitor.reduce as super

    inherit [_] Visitors_runtime.option_monoid

    method merge x _ = x

    method! on_expr env ((ty, pos, _) as expr) =
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
        super#on_expr env expr

    method! on_fun_param env fp =
      if
        Pos.exactly_matches_range
          fp.Aast.param_pos
          ~start_line:startl
          ~start_col:startc
          ~end_line:endl
          ~end_col:endc
      then
        Some (env, fp.Aast.param_annotation)
      else
        super#on_fun_param env fp

    method! on_class_id env ((ty, pos, _) as cid) =
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
        super#on_class_id env cid
  end

let type_at_pos_fused
    (ctx : Provider_context.t)
    (tast : Tast.program Tast_with_dynamic.t)
    (line_char_pairs : (int * int) list) : (Tast_env.env * Tast.ty) option list
    =
  (base_visitor ~human_friendly:false ~under_dynamic:false line_char_pairs)#go
    ctx
    tast.Tast_with_dynamic.under_normal_assumptions
  |> List.map ~f:(Option.map ~f:(fun (_, env, ty) -> (env, ty)))

let type_at_pos
    (ctx : Provider_context.t)
    (tast : Tast.program Tast_with_dynamic.t)
    (line : int)
    (char : int) : (Tast_env.env * Tast.ty) option =
  type_at_pos_fused ctx tast [(line, char)] |> function
  | [res] -> res
  | _ -> None

(* Return the expanded type of smallest expression at this
   position. Skips string literals in shape indexing expressions so
   hover results are more relevant.

   If [under_dynamic] is true, look for type produced
     when env.checked = CUnderDynamicAssumptions
   Otherwise, look for type produced in normal checking
     when env.checked = COnce or env.checked = CUnderNormalAssumptions.
*)
let human_friendly_type_at_pos
    ~under_dynamic
    (ctx : Provider_context.t)
    (tast : Tast.program Tast_with_dynamic.t)
    (line : int)
    (char : int) : (Tast_env.env * Tast.ty) option =
  let tast =
    if under_dynamic then
      Option.value ~default:[] tast.Tast_with_dynamic.under_dynamic_assumptions
    else
      tast.Tast_with_dynamic.under_normal_assumptions
  in
  (base_visitor ~human_friendly:true ~under_dynamic [(line, char)])#go ctx tast
  |> function
  | [Some (_, env, ty)] -> Some (env, Tast_expand.expand_ty env ty)
  | _ -> None

let type_at_range
    (ctx : Provider_context.t)
    (tast : Tast.program Tast_with_dynamic.t)
    (start_line : int)
    (start_char : int)
    (end_line : int)
    (end_char : int) : (Tast_env.env * Tast.ty) option =
  (range_visitor start_line start_char end_line end_char)#go
    ctx
    tast.Tast_with_dynamic.under_normal_assumptions

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
