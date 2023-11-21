(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Env = struct
  let incr_tp_depth t =
    let elab_wildcard_hint = t.Naming_phase_env.elab_wildcard_hint in
    let elab_wildcard_hint =
      Naming_phase_env.Elab_wildcard_hint.
        { elab_wildcard_hint with tp_depth = elab_wildcard_hint.tp_depth + 1 }
    in
    Naming_phase_env.{ t with elab_wildcard_hint }

  let reset_tp_depth t =
    let elab_wildcard_hint = t.Naming_phase_env.elab_wildcard_hint in
    let elab_wildcard_hint =
      Naming_phase_env.Elab_wildcard_hint.
        { elab_wildcard_hint with tp_depth = 0 }
    in
    Naming_phase_env.{ t with elab_wildcard_hint }

  let allow_wildcard
      Naming_phase_env.
        { elab_wildcard_hint = Elab_wildcard_hint.{ allow_wildcard; _ }; _ } =
    allow_wildcard

  let set_allow_wildcard t ~allow_wildcard =
    let elab_wildcard_hint = t.Naming_phase_env.elab_wildcard_hint in
    let elab_wildcard_hint =
      Naming_phase_env.Elab_wildcard_hint.
        { elab_wildcard_hint with allow_wildcard }
    in
    Naming_phase_env.{ t with elab_wildcard_hint }

  let tp_depth
      Naming_phase_env.
        { elab_wildcard_hint = Elab_wildcard_hint.{ tp_depth; _ }; _ } =
    tp_depth
end

let on_expr_ expr_ ~ctx =
  let ctx =
    match expr_ with
    | Aast.Cast _ -> Env.incr_tp_depth ctx
    | Aast.(Is _ | As _) -> Env.set_allow_wildcard ctx ~allow_wildcard:true
    | Aast.Upcast _ -> Env.set_allow_wildcard ctx ~allow_wildcard:false
    | _ -> ctx
  in
  (ctx, Ok expr_)

let on_targ targ ~ctx =
  (Env.set_allow_wildcard ~allow_wildcard:true @@ Env.incr_tp_depth ctx, Ok targ)

let on_hint_ hint_ ~ctx =
  let ctx =
    match hint_ with
    | Aast.(
        ( Hunion _ | Hintersection _ | Hoption _ | Hlike _ | Hsoft _
        | Hrefinement _ )) ->
      Env.reset_tp_depth ctx
    | Aast.(Htuple _ | Happly _ | Habstr _ | Hvec_or_dict _) ->
      Env.incr_tp_depth ctx
    | _ -> ctx
  in
  (ctx, Ok hint_)

let on_shape_field_info sfi ~ctx = (Env.incr_tp_depth ctx, Ok sfi)

let on_context on_error hint ~ctx =
  match hint with
  | (pos, Aast.Hwildcard) ->
    on_error
      (Naming_phase_error.naming @@ Naming_error.Invalid_wildcard_context pos);
    (ctx, Error (pos, Aast.Herr))
  | _ -> (ctx, Ok hint)

let on_hint on_error hint ~ctx =
  match hint with
  | (pos, Aast.Hwildcard) ->
    if Env.(allow_wildcard ctx && tp_depth ctx >= 1) (* prevents 3 as _ *) then
      (ctx, Ok hint)
    else
      let err =
        Naming_phase_error.naming @@ Naming_error.Wildcard_hint_disallowed pos
      in
      on_error err;
      (ctx, Ok (pos, Aast.Herr))
  | _ -> (ctx, Ok hint)

let on_ty_pat_refinement pr ~ctx =
  (Env.set_allow_wildcard ctx ~allow_wildcard:true, Ok pr)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_expr_ = Some on_expr_;
        on_ty_shape_field_info = Some on_shape_field_info;
        on_ty_targ = Some on_targ;
        on_ty_context = Some (fun elem ~ctx -> on_context on_error elem ~ctx);
        on_ty_hint_ = Some on_hint_;
        on_ty_hint = Some (fun elem ~ctx -> on_hint on_error elem ~ctx);
        on_ty_pat_refinement = Some on_ty_pat_refinement;
      }
