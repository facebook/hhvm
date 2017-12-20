(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Return the type of the smallest typed expression node whose associated span
 * (the Pos.t in its Tast.ExprAnnotation.t) contains the given position.
 * "Smallest" here refers to the size of the node's associated span, in terms of
 * its byte length in the original source file. "Typed" means that the Tast.ty
 * option in the expression's Tast.ExprAnnotation.t is not None.
 *
 * If the smallest typed expression containing the given position has a function
 * type and is being invoked in a Call expression, return that function's return
 * type rather than the type of the function (i.e., the type of the expression
 * returned by the Call expression).
 *
 * If there is no single smallest node (i.e., multiple typed expression nodes
 * have spans of the same length containing the given position, where that
 * length is less than the length of all other spans containing the given
 * position), return the type of the first of these nodes visited in a preorder
 * traversal of the Tast.
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
let type_at_pos tast line char =
  let open Option.Monad_infix in
  let visitor = object (self)
    inherit [_] Tast_visitor.reduce as super
    inherit [Pos.t * _ * _] Ast.option_monoid

    method private merge lhs rhs =
      (* A node with position P is not always a parent of every other node with
       * a position contained by P. Some desugaring can cause nodes to be
       * rearranged so that this is no longer the case (e.g., `invariant`).
       *
       * To deal with this, we simply take the smaller node. *)
      let lpos, _, _ = lhs in
      let rpos, _, _ = rhs in
      if Pos.length lpos <= Pos.length rpos then lhs else rhs

    method! on_expr env e =
      let (pos, ty) = fst e in
      let acc =
        if Pos.inside pos line char
        then ty >>| fun ty -> pos, self#saved_env, ty
        else None
      in
      self#plus acc (super#on_expr env e)

    (* When the expression being applied has a Tfun type, replace that type with
     * its return type. This matches with legacy behavior and is better-suited
     * for IDE hover (at present, since full function types are presented in a
     * way which makes them difficult to read). *)
    method! on_Call env ct e hl el uel =
      let return_type ty =
        let open Typing_defs in
        match snd ty with Tfun ft -> ft.ft_ret | _ -> ty
      in
      let (receiver_pos, _) = fst e in
      if Pos.inside receiver_pos line char
      then super#on_expr env e >>| fun (p, env, ty) -> p, env, return_type ty
      else super#on_Call env ct e hl el uel
  end in
  visitor#on_program () tast >>| fun (_, env, ty) -> env, ty

let make_result tcopt (saved_env, ty) =
  let file = Relative_path.create Relative_path.Dummy "<file>" in
  let env = Typing_env.empty tcopt file ~droot:None in
  let env = Tast_expand.restore_saved_env env saved_env in
  Typing_print.full_strip_ns env ty,
  Typing_print.to_json env ty |> Hh_json.json_to_string

let go:
  ServerEnv.env ->
  (ServerUtils.file_input * int * int) ->
  (string * string) option =
fun env pos ->
  let file, line, char = pos in
  let {ServerEnv.tcopt; files_info; _} = env in
  let _, tast = ServerIdeUtils.check_file_input tcopt files_info file in
  type_at_pos tast line char
  |> Option.map ~f:(make_result tcopt)
