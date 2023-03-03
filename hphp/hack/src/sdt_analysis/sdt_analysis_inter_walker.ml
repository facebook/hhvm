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

let create_custom_inter ~classish_kind_opt ~hierarchy_for_final_item =
  H.CustomInterConstraint
    CustomInterConstraint.{ classish_kind_opt; hierarchy_for_final_item }

let get_hierarchy_for_final_class ctx A.{ c_name = (_, sid); c_final; _ } =
  if not c_final then
    None
  else
    let open Option.Monad_infix in
    Decl_provider.get_class ctx sid >>| Decl_provider.Class.all_ancestor_names

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
      let ctx = Tast_env.get_ctx env in
      let id = H.Id.ClassLike sid in

      let at_inherits acc (_, hint_) =
        match hint_ with
        | A.Happly ((hack_pos, parent_sid), _hints) ->
          let parent_id = H.Id.ClassLike parent_sid in
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
        let hierarchy_for_final_item =
          get_hierarchy_for_final_class ctx class_
        in
        WalkResult.add_constraint
          wr
          id
          {
            origin = __LINE__;
            hack_pos = c_pos;
            decorated_data =
              create_custom_inter
                ~classish_kind_opt:(Some c_kind)
                ~hierarchy_for_final_item;
          }
      in
      WalkResult.(wr @ super#on_class_ env class_)

    method! on_fun_def env (A.{ fd_name = (f_pos, sid); _ } as fd) =
      let id = H.Id.Function sid in

      let hierarchy_for_final_item =
        Some
          (* `Some` because a top-level function is morally "final" (they cannot be extended) *)
          []
        (* [] because a top-level function doesn't inherit from anything *)
      in
      let wr =
        WalkResult.singleton
          id
          {
            origin = __LINE__;
            hack_pos = f_pos;
            decorated_data =
              create_custom_inter
                ~classish_kind_opt:None
                ~hierarchy_for_final_item;
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
