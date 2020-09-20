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
module Lattice = Ifc_security_lattice
module T = Typing_defs
module TClass = Decl_provider.Class
module TEnv = Typing_env
module TPhase = Typing_phase
module TReason = Typing_reason
module TUtils = Typing_utils

(* Functions to turn localized types into policied types. *)

exception LiftError of string

let fail fmt = Format.kasprintf (fun s -> raise (LiftError s)) fmt

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

let class_ty ?lump renv name =
  Tclass
    {
      c_name = name;
      c_self = get_policy lump renv ~prefix:name;
      c_lump = get_policy lump renv ~prefix:"lump";
    }

(* Turns a locl_ty into a type with policy annotations;
   the policy annotations are fresh policy variables *)
let rec ty ?prefix ?lump renv (t : T.locl_ty) =
  let ty = ty ?prefix ?lump renv in
  match T.get_node t with
  | T.Tprim _ -> Tprim (get_policy ?prefix lump renv)
  | T.Tgeneric (_name, _targs) ->
    (* TODO(T69551141) Handle type arguments *)
    Tgeneric (get_policy ?prefix lump renv)
  | T.Ttuple tyl -> Ttuple (List.map ~f:ty tyl)
  | T.Tunion tyl -> Tunion (List.map ~f:ty tyl)
  | T.Tintersection tyl -> Tinter (List.map ~f:ty tyl)
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
  | T.Tdependent (T.DTthis, tbound) ->
    (* TODO(T72024862): This treatment ignores late static binding. *)
    ty tbound
  | T.Toption t ->
    let tnull = Tprim (get_policy ?prefix lump renv) in
    Tunion [tnull; ty t]
  (* ---  types below are not yet supported *)
  | T.Tdependent (_, _ty) -> fail "Tdependent"
  | T.Tdarray (_keyty, _valty) -> fail "Tdarray"
  | T.Tvarray _ty -> fail "Tvarray"
  | T.Tvarray_or_darray (_keyty, _valty) -> fail "Tvarray_or_darray"
  | T.Tany _sentinel -> fail "Tany"
  | T.Terr -> fail "Terr"
  | T.Tnonnull -> fail "Tnonnull"
  | T.Tdynamic -> fail "Tdynamic"
  | T.Tshape (_sh_kind, _sh_type_map) -> fail "Tshape"
  | T.Tnewtype (_name, _ty_list, _as_bound) -> fail "Tnewtype"
  | T.Tobject -> fail "Tobject"
  | T.Tpu (_locl_ty, _sid) -> fail "Tpu"
  | T.Tpu_type_access (_sid1, _sid2) -> fail "Tpu_type_access"
  | T.Tunapplied_alias _ -> fail "Tunapplied_alias"
