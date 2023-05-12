(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Env = struct
  let like_type_hints_enabled Naming_phase_env.{ like_type_hints_enabled; _ } =
    like_type_hints_enabled

  let allow_like
      Naming_phase_env.
        { validate_like_hint = Validate_like_hint.{ allow_like }; _ } =
    allow_like

  let set_allow_like t ~allow_like =
    Naming_phase_env.
      { t with validate_like_hint = Validate_like_hint.{ allow_like } }
end

let on_expr_ expr_ ~ctx =
  let ctx =
    match expr_ with
    (* We reject likes in these constructs with a different error *)
    | Aast.(Is _ | As _ | Upcast _) -> Env.set_allow_like ctx ~allow_like:true
    | _ -> ctx
  in
  (ctx, Ok expr_)

let on_hint on_error hint ~ctx =
  let (err_opt, ctx) =
    match hint with
    | (pos, Aast.Hlike _)
      when not (Env.allow_like ctx || Env.like_type_hints_enabled ctx) ->
      (Some (Naming_phase_error.like_type pos), ctx)
    | (_, Aast.(Hfun _ | Happly _ | Haccess _ | Habstr _ | Hvec_or_dict _)) ->
      (None, Env.set_allow_like ctx ~allow_like:false)
    | _ -> (None, ctx)
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok hint)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_expr_ = Some on_expr_;
        on_ty_hint = Some (on_hint on_error);
      }
