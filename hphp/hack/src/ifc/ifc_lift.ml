(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ifc_types
module Decl = Ifc_decl
module Env = Ifc_env
module T = Typing_defs
module TClass = Decl_provider.Class
module TReason = Typing_reason

let fail fmt = Format.kasprintf (fun s -> raise (IFCError (LiftError s))) fmt

let expand_var renv id =
  (* Drops the environment. Var expansion only compresses paths, so this is
   * safe (but less efficient than threading the environment). *)
  let (_, ty) =
    Typing_inference_env.expand_var
      renv.re_tenv.Tast.inference_env
      Typing_reason.Rnone
      id
  in
  ty

(* Returns the lump policy if there is one in effect, otherwise
   generates a fresh policy variable *)
let get_policy ?prefix lump renv =
  match lump with
  | Some policy -> policy
  | None ->
    let prefix = Option.value prefix ~default:"v" in
    Env.new_policy_var renv prefix

let class_ty ?prefix ?lump renv name =
  let prefix = Option.value ~default:name prefix in
  Tclass
    {
      c_name = name;
      c_self = get_policy lump renv ~prefix;
      c_lump = get_policy lump renv ~prefix:(prefix ^ "_lump");
    }

(* Turns a locl_ty into a type with policy annotations;
   the policy annotations are fresh policy variables *)
let rec ty ?prefix ?lump renv (t : T.locl_ty) =
  let ty = ty ?prefix ?lump renv in
  match T.get_node t with
  | T.Tprim Aast.Tnull -> Tnull (get_policy ?prefix lump renv)
  | T.Tprim _ -> Tprim (get_policy ?prefix lump renv)
  | T.Tnonnull ->
    let pself = get_policy ?prefix lump renv
    and plump = get_policy ?prefix lump renv in
    Tnonnull (pself, plump)
  | T.Tdynamic -> Tdynamic (get_policy ?prefix lump renv)
  | T.Tgeneric (_name, _targs) ->
    (* TODO(T69551141) Handle type arguments *)
    Tgeneric (get_policy ?prefix lump renv)
  | T.Tclass ((_, "\\HH\\Pair"), _, tyl)
  | T.Ttuple tyl ->
    Ttuple (List.map ~f:ty tyl)
  | T.Tunion tyl -> Tunion (List.map ~f:ty tyl)
  | T.Tintersection tyl -> Tinter (List.map ~f:ty tyl)
  | T.Tvarray element_ty
  | T.Tclass ((_, "\\HH\\ConstSet"), _, [element_ty])
  | T.Tclass ((_, "\\HH\\ConstVector"), _, [element_ty])
  | T.Tclass ((_, "\\HH\\ImmSet"), _, [element_ty])
  | T.Tclass ((_, "\\HH\\ImmVector"), _, [element_ty])
  | T.Tclass ((_, "\\HH\\vec"), _, [element_ty]) ->
    Tcow_array
      {
        a_kind = Avec;
        (* Inventing a policy type for indices out of thin air *)
        a_key = Tprim (get_policy ~prefix:"key" lump renv);
        a_value = ty element_ty;
        a_length = get_policy ~prefix:"len" lump renv;
      }
  | T.Tvarray_or_darray (key_ty, value_ty)
  | T.Tvec_or_dict (key_ty, value_ty)
  | T.Tdarray (key_ty, value_ty)
  | T.Tclass ((_, "\\HH\\ConstMap"), _, [key_ty; value_ty])
  | T.Tclass ((_, "\\HH\\ImmMap"), _, [key_ty; value_ty])
  | T.Tclass ((_, "\\HH\\dict"), _, [key_ty; value_ty]) ->
    Tcow_array
      {
        a_kind = Adict;
        a_key = ty key_ty;
        a_value = ty value_ty;
        a_length = get_policy ~prefix:"len" lump renv;
      }
  | T.Tclass ((_, "\\HH\\keyset"), _, [value_ty]) ->
    let element_pty = ty value_ty in
    Tcow_array
      {
        a_kind = Akeyset;
        (* Keysets have identical keys and values with identity
            $keyset[$ix] === $ix (as bizarre as it is) *)
        a_key = element_pty;
        a_value = element_pty;
        a_length = get_policy ~prefix:"len" lump renv;
      }
  | T.Tclass ((_, name), _, targs) when String.equal name Decl.awaitable_id ->
    begin
      match targs with
      (* NOTE: Strip Awaitable out of the type since it has no affect on
         information flow *)
      | [inner_ty] -> ty inner_ty
      | _ -> fail "Awaitable needs one type parameter"
    end
  | T.Tclass ((_, name), _, _) -> class_ty ?lump renv name
  | T.Tvar id -> ty (expand_var renv id)
  | T.Tfun fun_ty ->
    Tfun
      {
        f_pc = get_policy ~prefix:"pc" lump renv;
        f_self = get_policy ?prefix lump renv;
        f_args =
          List.map ~f:(fun p -> ty p.T.fp_type.T.et_type) fun_ty.T.ft_params;
        f_ret = ty fun_ty.T.ft_ret.T.et_type;
        f_exn = class_ty ?lump renv Decl.exception_id;
      }
  | T.Toption t ->
    let tnull = Tnull (get_policy ?prefix lump renv) in
    Tunion [tnull; ty t]
  | T.Tshape (kind, fields) ->
    let lift sft =
      {
        sft_optional = sft.T.sft_optional;
        sft_policy = get_policy ?prefix lump renv;
        sft_ty = ty sft.T.sft_ty;
      }
    in
    let sh_kind =
      match kind with
      | Type.Open_shape ->
        let pself = get_policy ?prefix lump renv in
        let plump = get_policy ?prefix lump renv in
        let pnull = get_policy ?prefix lump renv in
        let tnull = Tnull pnull in
        let tmixed = Tunion [tnull; Tnonnull (pself, plump)] in
        Open_shape tmixed
      | Type.Closed_shape -> Closed_shape
    in
    Tshape { sh_kind; sh_fields = Typing_defs.TShapeMap.map lift fields }
  (* ---  types below are not yet supported *)
  | T.Tdependent (_, _ty) -> fail "Tdependent"
  | T.Tany _sentinel -> fail "Tany"
  | T.Terr -> fail "Terr"
  | T.Tnewtype (_name, _ty_list, _as_bound) -> fail "Tnewtype"
  | T.Tobject -> fail "Tobject"
  | T.Taccess (_locl_ty, _ids) -> fail "Taccess"
  | T.Tunapplied_alias _ -> fail "Tunapplied_alias"
  | T.Tneg _ -> fail "Tneg"
