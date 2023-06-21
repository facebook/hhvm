(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *These 'Unbound module' errors can be fixed... [read more](https://fburl.com/ocaml_hack_and_flow)
 *)

open Hh_prelude
open Typing_defs
open Typing_env_types
module Cls = Decl_provider.Class
module Env = Typing_env
module SN = Naming_special_names

module DataType = struct
  (* Modelled after data types in HHVM. See hphp/runtime/base/datatype.h *)
  module Tag = struct
    type t =
      | DictData
      | VecData
      | KeysetData
      | StringData
      | ResourceData
      | BoolData
      | IntData
      | FloatData
      | NullData
      | ObjectData
    [@@deriving show, eq, ord]

    let name = function
      | DictData -> "dict"
      | VecData -> "vec"
      | KeysetData -> "keyset"
      | StringData -> "string"
      | ResourceData -> "resource"
      | BoolData -> "bool"
      | IntData -> "int"
      | FloatData -> "float"
      | NullData -> "null"
      | ObjectData -> "object"
  end

  module Map = struct
    include WrappedMap.Make (Tag)
  end

  module Set = struct
    include Caml.Set.Make (Tag)
  end

  type t = Set.t [@@deriving eq, ord]

  (** `mixed` should cover all possible data types.
    * Update if addition tags are added to [Tag.t] *)
  let mixed =
    Set.of_list
      [
        Tag.DictData;
        Tag.VecData;
        Tag.KeysetData;
        Tag.StringData;
        Tag.ResourceData;
        Tag.BoolData;
        Tag.IntData;
        Tag.FloatData;
        Tag.NullData;
        Tag.ObjectData;
      ]

  let prim_to_datatypes (prim : Aast.tprim) : t =
    let open Aast in
    match prim with
    | Tint -> Set.singleton Tag.IntData
    | Tnull -> Set.singleton Tag.NullData
    | Tvoid -> Set.singleton Tag.NullData
    | Tbool -> Set.singleton Tag.BoolData
    | Tfloat -> Set.singleton Tag.FloatData
    | Tstring -> Set.singleton Tag.StringData
    | Tresource -> Set.singleton Tag.ResourceData
    | Tnum -> Set.of_list [Tag.IntData; Tag.FloatData]
    | Tarraykey -> Set.of_list [Tag.IntData; Tag.StringData]
    | Tnoreturn -> Set.empty

  (* Set of interfaces that contain non-object members *)
  let special_interfaces =
    SSet.of_list
      [
        SN.Classes.cStringish;
        SN.Classes.cXHPChild;
        SN.Collections.cTraversable;
        SN.Collections.cKeyedTraversable;
        SN.Collections.cContainer;
        SN.Collections.cKeyedContainer;
      ]

  (** The [special_interfaces] contain additional data type tags beside
    * [Tag.ObjectData]. These special interfaces are handled in [Typing_subtype].
    * To avoid duplicating the logic, we check if a certain tag should be included
    * by doing a subtype test against the interface type. If the type
    * corresponding to a tag is a subtype then it will be included in the list.
    *)
  let special_interface_to_datatypes env interface_ty =
    let open Typing_make_type in
    let r = Typing_reason.none in
    let tag_to_type =
      [
        (Tag.DictData, dict r (nothing r) (nothing r));
        (Tag.VecData, vec r (nothing r));
        (Tag.KeysetData, keyset r (nothing r));
        (Tag.StringData, string r);
        (Tag.ResourceData, resource r);
        (Tag.BoolData, bool r);
        (Tag.IntData, int r);
        (Tag.FloatData, float r);
        (Tag.NullData, null r);
      ]
    in
    List.fold
      tag_to_type
      ~init:Set.empty
      ~f:
        begin
          fun acc (tag, tag_ty) ->
            if Typing_utils.is_sub_type env tag_ty interface_ty then
              Set.add tag acc
            else
              acc
        end

  let union (left : t) (right : t) : t = Set.union left right

  and intersect (left : t) (right : t) : t = Set.inter left right

  and minus (left : t) (right : t) : t = Set.diff left right

  let rec fromTy (env : env) (ty : locl_ty) : t =
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Tprim prim -> prim_to_datatypes prim
    | Tnonnull -> minus mixed @@ Set.singleton Tag.NullData
    | Tdynamic -> mixed
    | Tany _ -> mixed
    | Toption ty -> fromTy env ty |> union @@ Set.singleton Tag.NullData
    (* For now say it has the same tags as `nonnull`.
     * We should be able to be more precise, but need to
     * validate what are all the data types that are valid callables *)
    | Tfun _ -> fromTy env (mk (get_reason ty, Tnonnull))
    | Ttuple _ -> Set.singleton Tag.VecData
    | Tshape _ -> Set.singleton Tag.DictData
    | Tvar _ -> mixed
    | Tgeneric (name, tyl) ->
      let upper_bounds = Env.get_upper_bounds env name tyl in
      Typing_set.fold
        (fun ty acc -> intersect acc @@ fromTy env ty)
        upper_bounds
        mixed
    | Tunion tyl ->
      List.fold tyl ~init:Set.empty ~f:(fun acc ty ->
          union acc @@ fromTy env ty)
    | Tintersection tyl ->
      List.fold tyl ~init:mixed ~f:(fun acc ty ->
          intersect acc @@ fromTy env ty)
    | Tvec_or_dict _ -> Set.of_list [Tag.VecData; Tag.DictData]
    | Taccess (root_ty, id) ->
      let ety_env = empty_expand_env in
      let ((env, _), ty) =
        Typing_utils.expand_typeconst
          ety_env
          env
          root_ty
          id
          ~allow_abstract_tconst:true
      in
      fromTy env ty
    | Tdependent (_, ty) -> fromTy env ty
    | Tnewtype (name, tyl, ty) ->
      let (env, ty) =
        match Env.get_typedef env name with
        (* When determining the datatype associated with a type we should
         * expand the case type instead of looking at the upper bound.
         * If we do not expand, then we will over approximate the datatype.
         * Consider:
         *
         *  case type Type1 = bool | int;
         *  case type Type2 = Type1 | string;
         *
         * If we do not expand we will reject the definition of `Type2`
         * because we will believe the datatype for `Type1` contains `string`.
         * By expanding we can allow this definition. *)
        | Some { td_type = variants; td_vis = Aast.CaseType; td_tparams; _ } ->
          let ((env, _ty_err_opt), variants) =
            (* The this_ty does not need to be set because case types cannot
             * appear within classes thus cannot us the this type.
             * If we ever change that this could needs to be changed *)
            Typing_phase.localize
              ~ety_env:
                {
                  empty_expand_env with
                  substs =
                    (if List.is_empty tyl then
                      SMap.empty
                    else
                      Decl_subst.make_locl td_tparams tyl);
                }
              env
              variants
          in
          (env, variants)
        | _ -> (env, ty)
      in
      fromTy env ty
    | Tunapplied_alias _ ->
      Typing_defs.error_Tunapplied_alias_in_illegal_context ()
    | Tclass ((_, cls), _, _) when String.equal cls SN.Collections.cDict ->
      Set.singleton Tag.DictData
    | Tclass ((_, cls), _, _) when String.equal cls SN.Collections.cKeyset ->
      Set.singleton Tag.KeysetData
    | Tclass ((_, cls), _, _) when String.equal cls SN.Collections.cVec ->
      Set.singleton Tag.VecData
    | Tclass ((_, cls), _, _) when String.equal cls SN.Collections.cAnyArray ->
      Set.of_list [Tag.DictData; Tag.KeysetData; Tag.VecData]
    | Tclass ((_, cls), _, _) when SSet.mem cls special_interfaces ->
      let tags = special_interface_to_datatypes env ty in
      Set.add Tag.ObjectData tags
    | Tclass (_, _, _) -> Set.singleton Tag.ObjectData
    | Tneg n ->
      let right =
        match n with
        | Neg_prim prim -> prim_to_datatypes prim
        | Neg_class (_, _) -> Set.singleton Tag.ObjectData
      in
      minus mixed right
end

let mk_data_type_mapping env variants =
  let f map variant =
    let ((env, _), ty) =
      Typing_phase.localize_hint_no_subst env ~ignore_errors:true variant
    in
    let tags = DataType.fromTy env ty in
    DataType.Set.fold
      (fun tag acc -> DataType.Map.add ~combine:( @ ) tag [variant] acc)
      tags
      map
  in
  List.fold variants ~init:DataType.Map.empty ~f
