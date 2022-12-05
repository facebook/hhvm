(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  type t = {
    allow_wildcard: bool;
    tp_depth: int;
  }

  let create ?(allow_wildcard = false) ?(tp_depth = 0) () =
    { allow_wildcard; tp_depth }

  let empty = { allow_wildcard = false; tp_depth = 0 }

  let incr_tp_depth t = { t with tp_depth = t.tp_depth + 1 }
end

let visitor =
  object (self)
    inherit [_] Naming_visitors.mapreduce as super

    method! on_Cast env hint expr =
      super#on_Cast Env.(incr_tp_depth env) hint expr

    method! on_Is env expr hint =
      super#on_Is Env.{ env with allow_wildcard = true } expr hint

    method! on_As env expr hint is_final =
      super#on_As Env.{ env with allow_wildcard = true } expr hint is_final

    method! on_Upcast env expr hint =
      super#on_Upcast Env.{ env with allow_wildcard = false } expr hint

    method! on_targ _ targ =
      super#on_targ Env.(create ~allow_wildcard:true ~tp_depth:1 ()) targ

    method! on_shape_field_info env sfi =
      super#on_shape_field_info Env.(incr_tp_depth env) sfi

    method! on_Hunion Env.{ allow_wildcard; _ } hints =
      super#on_Hunion Env.(create ~allow_wildcard ()) hints

    method! on_Hintersection Env.{ allow_wildcard; _ } hints =
      super#on_Hintersection Env.(create ~allow_wildcard ()) hints

    method! on_Htuple Env.{ allow_wildcard; tp_depth } hints =
      super#on_Htuple
        Env.(create ~allow_wildcard ~tp_depth:(tp_depth + 1) ())
        hints

    method! on_Hoption Env.{ allow_wildcard; _ } hint =
      super#on_Hoption Env.(create ~allow_wildcard ()) hint

    method! on_Hlike Env.{ allow_wildcard; _ } hint =
      super#on_Hlike Env.(create ~allow_wildcard ()) hint

    method! on_Hsoft Env.{ allow_wildcard; _ } hint =
      super#on_Hsoft Env.(create ~allow_wildcard ()) hint

    method! on_Happly Env.{ allow_wildcard; tp_depth } tycon hints =
      super#on_Happly
        Env.(create ~allow_wildcard ~tp_depth:(tp_depth + 1) ())
        tycon
        hints

    method! on_Habstr Env.{ allow_wildcard; tp_depth } name hints =
      super#on_Habstr
        Env.(create ~allow_wildcard ~tp_depth:(tp_depth + 1) ())
        name
        hints

    method! on_Hvec_or_dict Env.{ allow_wildcard; tp_depth } hk hv =
      super#on_Hvec_or_dict
        Env.(create ~allow_wildcard ~tp_depth:(tp_depth + 1) ())
        hk
        hv

    method! on_Hrefinement Env.{ allow_wildcard; _ } subject members =
      super#on_Hrefinement Env.(create ~allow_wildcard ()) subject members

    method! on_context env hint =
      let res =
        match hint with
        | (pos, Aast.Happly ((_, tycon_name), _))
          when String.equal tycon_name SN.Typehints.wildcard ->
          Error
            ( (pos, Aast.Herr),
              Err.naming @@ Naming_error.Invalid_wildcard_context pos )
        | _ -> Ok hint
      in
      match res with
      | Ok hint -> self#on_hint env hint
      | Error (hint, err) -> (hint, err)

    method! on_hint (Env.{ allow_wildcard; tp_depth } as env) hint =
      let res =
        match hint with
        | (pos, Aast.Happly ((_, tycon_name), hints))
          when String.equal tycon_name SN.Typehints.wildcard ->
          if allow_wildcard && tp_depth >= 1 (* prevents 3 as _ *) then
            if not (List.is_empty hints) then
              let err =
                Err.naming
                @@ Naming_error.Tparam_applied_to_type
                     { pos; tparam_name = SN.Typehints.wildcard }
              in
              Error ((pos, Aast.Herr), err)
            else
              Ok hint
          else
            let err = Err.naming @@ Naming_error.Wildcard_hint_disallowed pos in
            Error ((pos, Aast.Herr), err)
        | _ -> Ok hint
      in
      match res with
      | Ok hint -> super#on_hint env hint
      | Error (hint, err) -> (hint, err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
