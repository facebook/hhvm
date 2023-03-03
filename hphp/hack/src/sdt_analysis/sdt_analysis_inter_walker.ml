(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Sdt_analysis_types
module A = Aast

let syntactically_nadable =
  H.CustomInterConstraint CustomInterConstraint.SyntacticallyNadable

let is_syntactically_nadable =
  Ast_defs.(
    function
    | Cclass _
    | Cinterface
    | Ctrait ->
      true
    | Cenum
    | Cenum_class _ ->
      false)

let reduce_walk_result =
  object
    inherit
      [H.inter_constraint_ decorated WalkResult.t] Tast_visitor.reduce as super

    method zero = WalkResult.empty

    method plus = WalkResult.( @ )

    method! on_class_
        env
        (A.
           {
             c_name = (c_pos, sid);
             c_extends;
             c_uses;
             c_implements;
             c_reqs;
             c_kind;
             _;
           } as class_) =
      let id = H.ClassLike sid in

      let at_inherits acc (_, hint_) =
        match hint_ with
        | A.Happly ((hack_pos, parent_sid), _hints) ->
          let parent_id = H.ClassLike parent_sid in
          let constraint_ = H.Inherits parent_id in
          let decorated =
            { origin = __LINE__; hack_pos; decorated_data = constraint_ }
          in
          WalkResult.add_constraint acc id decorated
        | _ -> acc
      in
      let inherited =
        c_uses @ c_extends @ c_implements @ List.map ~f:fst c_reqs
      in
      let wr = List.fold ~init:WalkResult.empty ~f:at_inherits inherited in
      let wr = WalkResult.add_id wr id in
      let wr =
        if is_syntactically_nadable c_kind then
          WalkResult.add_constraint
            wr
            id
            {
              origin = __LINE__;
              hack_pos = c_pos;
              decorated_data = syntactically_nadable;
            }
        else
          wr
      in
      WalkResult.(wr @ super#on_class_ env class_)

    method! on_fun_def env (A.{ fd_name = (f_pos, sid); _ } as fd) =
      let id = H.Function sid in
      let wr =
        WalkResult.singleton
          id
          {
            origin = __LINE__;
            hack_pos = f_pos;
            decorated_data = syntactically_nadable;
          }
      in
      let wr = WalkResult.add_id wr id in
      WalkResult.(wr @ super#on_fun_def env fd)
  end

let program (ctx : Provider_context.t) (tast : Tast.program) :
    H.inter_constraint_ decorated WalkResult.t =
  let def acc def =
    let tast_env = Tast_env.def_env ctx def in
    WalkResult.(acc @ reduce_walk_result#on_def tast_env def)
  in
  List.fold ~init:WalkResult.empty ~f:def tast
