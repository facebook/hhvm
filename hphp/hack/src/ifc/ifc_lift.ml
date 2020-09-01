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

(* Empty type environment useful for localising decl types and pretty printing types *)
let empty_tenv meta = TEnv.empty meta.m_ctx meta.m_path None

let expand_var proto_renv id =
  (* Drops the environment. Var expansion only compresses paths, so this is
   * safe (but less efficient than threading the environment). *)
  let (_, ty) =
    Typing_inference_env.expand_var
      proto_renv.pre_tenv.Tast.inference_env
      Typing_reason.Rnone
      id
  in
  ty

(* If there is a lump policy variable in effect, return that otherwise
   generate a new policy variable. *)
let get_policy ?prefix lump_pol_opt proto_renv =
  match lump_pol_opt with
  | Some lump_pol -> lump_pol
  | None ->
    let prefix = Option.value prefix ~default:"v" in
    Env.new_policy_var proto_renv prefix

let rec class_ty lump_pol_opt proto_renv name =
  let { cd_policied_properties } =
    match SMap.find_opt name proto_renv.pre_decl.de_class with
    | Some class_sig -> class_sig
    | None -> fail "could not found a class policy signature for %s" name
  in
  let prop_pol { pp_name; pp_purpose; pp_pos; _ } =
    (* Purpose of the property takes precedence over any lump policy.
     * TODO(T74471162): Lump might itself be a purpose. There should be a check
     * or constraint generation of some sort.
     *)
    let parse_policy = Lattice.parse_policy (PosSet.singleton pp_pos) in
    let pp_policy = parse_policy pp_purpose in
    (pp_name, pp_policy)
  in
  Tclass
    {
      c_name = name;
      c_self = get_policy lump_pol_opt proto_renv ~prefix:name;
      c_lump = get_policy lump_pol_opt proto_renv ~prefix:"lump";
      c_properties = SMap.of_list (List.map ~f:prop_pol cd_policied_properties);
    }

(* Turns a locl_ty into a type with policy annotations;
   the policy annotations are fresh policy variables *)
and ty ?prefix lump_pol_opt proto_renv (t : T.locl_ty) =
  let ty = ty ?prefix lump_pol_opt proto_renv in
  match T.get_node t with
  | T.Tprim _ -> Tprim (get_policy lump_pol_opt proto_renv ?prefix)
  | T.Tgeneric (_name, _targs) ->
    (* TODO(T69551141) Handle type arguments *)
    Tgeneric (get_policy lump_pol_opt proto_renv ?prefix)
  | T.Ttuple tyl -> Ttuple (List.map ~f:ty tyl)
  | T.Tunion tyl -> Tunion (List.map ~f:ty tyl)
  | T.Tintersection tyl -> Tinter (List.map ~f:ty tyl)
  | T.Tclass ((_, name), _, _) -> class_ty lump_pol_opt proto_renv name
  | T.Tvar id -> ty (expand_var proto_renv id)
  | T.Tfun fun_ty ->
    Tfun
      {
        f_pc = get_policy lump_pol_opt proto_renv ~prefix:"pc";
        f_self = get_policy lump_pol_opt proto_renv ?prefix;
        f_args =
          List.map ~f:(fun p -> ty p.T.fp_type.T.et_type) fun_ty.T.ft_params;
        f_ret = ty fun_ty.T.ft_ret.T.et_type;
        f_exn = class_ty None proto_renv Decl.exception_id;
      }
  | T.Tdependent (T.DTthis, tbound) ->
    (* TODO(T72024862): This treatment ignores late static binding. *)
    ty tbound
  (* ---  types below are not yet supported *)
  | T.Tdependent (_, _ty) -> fail "Tdependent"
  | T.Tdarray (_keyty, _valty) -> fail "Tdarray"
  | T.Tvarray _ty -> fail "Tvarray"
  | T.Tvarray_or_darray (_keyty, _valty) -> fail "Tvarray_or_darray"
  | T.Tany _sentinel -> fail "Tany"
  | T.Terr -> fail "Terr"
  | T.Tnonnull -> fail "Tnonnull"
  | T.Tdynamic -> fail "Tdynamic"
  | T.Toption _ty -> fail "Toption"
  | T.Tshape (_sh_kind, _sh_type_map) -> fail "Tshape"
  | T.Tnewtype (_name, _ty_list, _as_bound) -> fail "Tnewtype"
  | T.Tobject -> fail "Tobject"
  | T.Tpu (_locl_ty, _sid) -> fail "Tpu"
  | T.Tpu_type_access (_sid1, _sid2) -> fail "Tpu_type_access"
  | T.Tunapplied_alias _ -> fail "Tunapplied_alias"
