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
  type t = unit

  let empty = ()
end

let visitor =
  object (self)
    inherit [_] Naming_visitors.mapreduce as super

    method! on_expr_ env expr_ =
      let res =
        match expr_ with
        | Aast.Collection ((pos, cname), c_targ_opt, afields)
          when Nast.is_vc_kind cname ->
          let (targ_opt, targ_err) =
            match c_targ_opt with
            | Some (Aast.CollectionTV tv) -> (Some tv, self#zero)
            | Some (Aast.CollectionTKV _) ->
              (None, Err.naming @@ Naming_error.Too_many_arguments pos)
            | _ -> (None, self#zero)
          in
          let (exprs, fields_err) =
            super#on_list (self#afield_value cname) env afields
          in
          let vc_kind = Nast.get_vc_kind cname in
          Ok
            ( Aast.ValCollection (vc_kind, targ_opt, exprs),
              self#plus targ_err fields_err )
        | Aast.Collection ((pos, cname), c_targ_opt, afields)
          when Nast.is_kvc_kind cname ->
          let (targs_opt, targ_err) =
            match c_targ_opt with
            | Some (Aast.CollectionTKV (tk, tv)) -> (Some (tk, tv), self#zero)
            | Some (Aast.CollectionTV _) ->
              (None, Err.naming @@ Naming_error.Too_few_arguments pos)
            | _ -> (None, self#zero)
          in
          let (fields, fields_err) =
            super#on_list (self#afield_key_value cname) env afields
          in
          let kvc_kind = Nast.get_kvc_kind cname in
          Ok
            ( Aast.KeyValCollection (kvc_kind, targs_opt, fields),
              self#plus targ_err fields_err )
        | Aast.Collection ((pos, cname), _, [])
          when String.equal SN.Collections.cPair cname ->
          Error (pos, Err.naming @@ Naming_error.Too_few_arguments pos)
        | Aast.Collection ((pos, cname), c_targ_opt, [fst; snd])
          when String.equal SN.Collections.cPair cname ->
          let (targs_opt, targ_err) =
            match c_targ_opt with
            | Some (Aast.CollectionTKV (tk, tv)) -> (Some (tk, tv), self#zero)
            | Some (Aast.CollectionTV _) ->
              (None, Err.naming @@ Naming_error.Too_few_arguments pos)
            | _ -> (None, self#zero)
          in
          let (fst, fst_err) = self#afield_value SN.Collections.cPair env fst
          and (snd, snd_err) = self#afield_value SN.Collections.cPair env snd in
          let err = self#plus targ_err (self#plus fst_err snd_err) in
          Ok (Aast.Pair (targs_opt, fst, snd), err)
        | Aast.Collection ((pos, cname), _, _)
          when String.equal SN.Collections.cPair cname ->
          Error (pos, Err.naming @@ Naming_error.Too_many_arguments pos)
        | Aast.Collection ((pos, cname), _, _) ->
          Error
            (pos, Err.naming @@ Naming_error.Expected_collection { pos; cname })
        | _ -> Ok (expr_, self#zero)
      in
      match res with
      | Ok (expr_, err) ->
        let (expr_, super_err) = super#on_expr_ env expr_ in
        (expr_, self#plus err super_err)
      | Error (pos, err) -> (Err.invalid_expr_ pos, err)

    method private afield_value cname _env afield =
      match afield with
      | Aast.AFvalue e -> (e, self#zero)
      | Aast.AFkvalue (((_, pos, _) as e), _) ->
        (e, Err.naming @@ Naming_error.Unexpected_arrow { pos; cname })

    method private afield_key_value cname _env afield =
      match afield with
      | Aast.AFkvalue (ek, ev) -> ((ek, ev), self#zero)
      | Aast.AFvalue ((_, pos, _) as ek) ->
        let ev =
          ( (),
            pos,
            Aast.Lvar (pos, Local_id.make_unscoped "__internal_placeholder") )
        in
        ((ek, ev), Err.naming @@ Naming_error.Missing_arrow { pos; cname })
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
