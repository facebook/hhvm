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
module T = Typing_defs
module TClass = Decl_provider.Class
module TEnv = Typing_env
module TPhase = Typing_phase
module TReason = Typing_reason
module TUtils = Typing_utils

(* Functions to turn localized types into policied types. *)

exception LiftError of string

let fail fmt = Format.kasprintf (fun s -> raise (LiftError s)) fmt

let empty_tenv meta = TEnv.empty meta.m_ctx meta.m_path None

let find_ancestor_tparams proto_renv class_name (targs : T.locl_ty list) =
  let class_decl = Decl.find_core_class_decl proto_renv.pre_meta class_name in
  let tparam_names =
    let tp_name tparam = snd tparam.T.tp_name in
    List.map ~f:tp_name @@ TClass.tparams class_decl
  in
  let substitution =
    match List.zip tparam_names targs with
    | Some zipped -> SMap.of_list zipped
    | None ->
      fail "%s has uneven number of type parameters and arguments" class_name
  in
  let extract_tparams (ancestor_name, ancestor) =
    let tenv = empty_tenv proto_renv.pre_meta in
    (* Type localisation configuration. We use it for instantiating type parameters. *)
    let ety_env =
      let this_ty = T.mk (TReason.none, TUtils.this_of @@ TEnv.get_self tenv) in
      {
        T.type_expansions = [];
        substs = substitution;
        quiet = false;
        on_error =
          (fun ?code:_ _ ->
            fail "something went wrong during generic substitution");
        this_ty;
        from_class = None;
      }
    in
    let (_, ty) = TPhase.localize ~ety_env tenv ancestor in
    match T.get_node ty with
    | T.Tclass (_, _, targs) ->
      if List.is_empty targs then
        None
      else
        Some (ancestor_name, targs)
    | _ ->
      fail
        "Tried to extract type parameters of %s, but it is not a class"
        ancestor_name
  in
  TClass.all_ancestors class_decl
  |> List.filter_map ~f:extract_tparams
  |> SMap.of_list

(* If there is a lump policy variable in effect, return that otherwise
   generate a new policy variable. *)
let get_policy ?prefix lump_pol_opt proto_renv =
  match lump_pol_opt with
  | Some lump_pol -> lump_pol
  | None ->
    let prefix = Option.value prefix ~default:"v" in
    Ifc_env.new_policy_var proto_renv prefix

let rec class_ty lump_pol_opt proto_renv targs name =
  let { cd_policied_properties } =
    match SMap.find_opt name proto_renv.pre_decl.de_class with
    | Some class_sig -> class_sig
    | None -> fail "could not found a class policy signature for %s" name
  in
  let prop_ty { pp_name; pp_type; pp_purpose; pp_pos; _ } =
    (* Purpose of the property takes precedence over any lump policy. *)
    let lump_pol_opt =
      let pos = PosSet.singleton pp_pos in
      Option.merge
        (Option.map ~f:(Ifc_security_lattice.parse_policy pos) pp_purpose)
        lump_pol_opt
        ~f:(fun a _ -> a)
    in
    (pp_name, lazy (ty ~prefix:("." ^ pp_name) lump_pol_opt proto_renv pp_type))
  in
  let lump_pol = get_policy lump_pol_opt proto_renv ~prefix:"lump" in
  let c_tparams =
    let prefix = "tp" in
    let tparam_ty = ty ~prefix lump_pol_opt proto_renv in
    let tparams = find_ancestor_tparams proto_renv name targs in
    let tparams =
      if List.is_empty targs then
        tparams
      else
        SMap.add name targs tparams
    in
    SMap.map (List.map ~f:tparam_ty) tparams
  in
  Tclass
    {
      c_name = name;
      c_self = get_policy lump_pol_opt proto_renv ~prefix:name;
      c_lump = lump_pol;
      c_property_map = SMap.of_list (List.map ~f:prop_ty cd_policied_properties);
      c_tparams;
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
  | T.Tclass ((_, name), _, tparams) ->
    class_ty lump_pol_opt proto_renv tparams name
  | T.Tvar id ->
    (* Drops the environment `expand_var` returns. This is logically
     * correct, but threading the envrionment would lead to faster future
     * `Tvar` lookups.
     *)
    let (_, t) =
      Typing_inference_env.expand_var
        proto_renv.pre_tenv.Tast.inference_env
        Typing_reason.Rnone
        id
    in
    ty t
  | T.Tfun fun_ty ->
    Tfun
      {
        f_pc = get_policy lump_pol_opt proto_renv ~prefix:"pc";
        f_self = get_policy lump_pol_opt proto_renv ?prefix;
        f_args =
          List.map ~f:(fun p -> ty p.T.fp_type.T.et_type) fun_ty.T.ft_params;
        f_ret = ty fun_ty.T.ft_ret.T.et_type;
        f_exn = class_ty None proto_renv [] Ifc_decl.exception_id;
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
