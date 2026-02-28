(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env
module SN = Naming_special_names

(** Produce an error on (string) casts of objects. Currently it is allowed in HHVM to
    cast an object if it is Stringish (i.e., has a __toString() method), but all
    (string) casts of objects will be banned in the future. Eventually,
    __toString/(string) casts of objects will be removed from HHVM entirely. *)

let check__toString m =
  let (pos, name) = m.m_name in
  if String.equal name SN.Members.__toString then (
    if (not (Aast.equal_visibility m.m_visibility Public)) || m.m_static then
      Diagnostics.add_diagnostic
        Nast_check_error.(to_user_diagnostic @@ ToString_visibility pos);
    match hint_of_type_hint m.m_ret with
    | Some (_, Happly ((_, id), _))
    | Some (_, Hlike (_, Happly ((_, id), _)))
      when String.equal SN.Classes.cString id ->
      ()
    | Some (p, _) ->
      Diagnostics.add_diagnostic
        Nast_check_error.(to_user_diagnostic @@ ToString_returns_string p)
    | None -> ()
  )

let rec is_stringish env ty =
  let (env, ety) = Env.expand_type env ty in
  match get_node ety with
  | Toption ty' -> is_stringish env ty'
  | Tunion tyl -> List.for_all ~f:(is_stringish env) tyl
  | Tintersection tyl -> List.exists ~f:(is_stringish env) tyl
  | Tgeneric _
  | Tnewtype _
  | Tdependent _ ->
    let (env, tyl) = Env.get_concrete_supertypes ~abstract_enum:true env ty in
    List.for_all ~f:(is_stringish env) tyl
  | Tclass ((_, id), _, _) when String.equal SN.Classes.cString id -> true
  | Tclass (x, _, _) ->
    Option.is_none (Decl_entry.to_option @@ Env.get_class env (snd x))
  (* TODO akenn: error tyvar? *)
  | Tany _
  | Tdynamic
  | Tnonnull
  | Tprim _
  | Tneg _ ->
    true
  | Tvec_or_dict _
  | Tvar _
  | Ttuple _
  | Tfun _
  | Tshape _
  | Tlabel _
  | Taccess _ ->
    false
  | Tclass_ptr _ ->
    TypecheckerOptions.allow_class_string_cast (Env.get_tcopt env)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, p, expr) =
      match expr with
      | Cast ((_, Happly ((_, id), _)), te)
        when String.equal SN.Classes.cString id ->
        let (ty, _, _) = te in
        if not (is_stringish env ty) then
          let Equal = Tast_env.eq_typing_env in
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.String_cast
                   { pos = p; ty_name = lazy (Env.print_ty env ty) })
      | _ -> ()

    method! at_method_ _ m = check__toString m
  end
