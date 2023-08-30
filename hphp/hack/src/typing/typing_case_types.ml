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
module Env = Typing_env
module SN = Naming_special_names

(* Modelled after data types in HHVM. See hphp/runtime/base/datatype.h *)
module Tag = struct
  type ctx = unit

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
  [@@deriving eq]

  let name = function
    | DictData -> "Dict"
    | VecData -> "Vec"
    | KeysetData -> "Keyset"
    | StringData -> "String"
    | ResourceData -> "Resource"
    | BoolData -> "Boolean"
    | IntData -> "Int64"
    | FloatData -> "Double"
    | NullData -> "Null"
    | ObjectData -> "Object"

  let relation tag1 ~ctx:_ tag2 =
    let open ApproxSet.Set_relation in
    if equal tag1 tag2 then
      Equal
    else
      Disjoint

  let all_tags =
    [
      DictData;
      VecData;
      KeysetData;
      StringData;
      ResourceData;
      BoolData;
      IntData;
      FloatData;
      NullData;
      ObjectData;
    ]
end

module DataType = struct
  module Set = ApproxSet.Make (Tag)

  type t = Set.t

  (** `mixed` should cover all possible data types.
      Update [Tag.all_tags] if additional tags are added to [Tag.t] *)
  let mixed = Set.of_list Tag.all_tags

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

  module Class = struct
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
     [Tag.ObjectData]. These special interfaces are handled in [Typing_subtype].
     To avoid duplicating the logic, we check if a certain tag should be included
     by doing a subtype test against the interface type. If the type
     corresponding to a tag is a subtype then it will be included in the list.
    *)
    let special_interface_to_datatypes env interface =
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
      let (env, interface_ty) =
        match interface with
        | name
          when String.equal name SN.Collections.cTraversable
               || String.equal name SN.Collections.cContainer ->
          let (env, targ) = Env.fresh_type env Pos.none in
          let ty = Typing_make_type.class_type Reason.none name [targ] in
          (env, ty)
        | name
          when String.equal name SN.Collections.cKeyedTraversable
               || String.equal name SN.Collections.cKeyedContainer ->
          let (env, targ1) = Env.fresh_type env Pos.none in
          let (env, targ2) = Env.fresh_type env Pos.none in
          let ty =
            Typing_make_type.class_type Reason.none name [targ1; targ2]
          in
          (env, ty)
        | name
          when String.equal name SN.Classes.cStringish
               || String.equal name SN.Classes.cXHPChild ->
          (env, Typing_make_type.class_type Reason.none name [])
        | name ->
          failwithf
            "Unexpected special interface `%s` when determining data types"
            name
            ()
      in
      let tags =
        List.fold
          tag_to_type
          ~init:[]
          ~f:
            begin
              fun acc (tag, tag_ty) ->
                if Typing_utils.can_sub_type env tag_ty interface_ty then
                  tag :: acc
                else
                  acc
            end
      in
      Set.of_list tags

    let special_interface_cache : t String.Table.t = String.Table.create ()

    let to_datatypes (env : env) cls : t =
      match cls with
      | cls when String.equal cls SN.Collections.cDict ->
        Set.singleton Tag.DictData
      | cls when String.equal cls SN.Collections.cKeyset ->
        Set.singleton Tag.KeysetData
      | cls when String.equal cls SN.Collections.cVec ->
        Set.singleton Tag.VecData
      | cls when String.equal cls SN.Collections.cAnyArray ->
        Set.of_list [Tag.DictData; Tag.KeysetData; Tag.VecData]
      | cls when SSet.mem cls special_interfaces ->
        String.Table.find_or_add special_interface_cache cls ~default:(fun () ->
            let tags = special_interface_to_datatypes env cls in
            Set.union (Set.singleton Tag.ObjectData) tags)
      | _ -> Set.singleton Tag.ObjectData
  end

  let rec fromTy (env : env) (ty : locl_ty) : t =
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Tprim prim -> prim_to_datatypes prim
    | Tnonnull -> Set.diff mixed @@ Set.singleton Tag.NullData
    | Tdynamic -> mixed
    | Tany _ -> mixed
    | Toption ty -> fromTy env ty |> Set.union @@ Set.singleton Tag.NullData
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
        (fun ty acc -> Set.inter acc @@ fromTy env ty)
        upper_bounds
        mixed
    | Tunion tyl ->
      List.fold tyl ~init:Set.empty ~f:(fun acc ty ->
          Set.union acc @@ fromTy env ty)
    | Tintersection tyl ->
      List.fold tyl ~init:mixed ~f:(fun acc ty ->
          Set.inter acc @@ fromTy env ty)
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
            Typing_utils.localize
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
    | Tclass ((_, cls), _, _) -> Class.to_datatypes env cls
    | Tneg n ->
      let right =
        match n with
        | Neg_prim prim -> prim_to_datatypes prim
        | Neg_class (_, cls) -> Class.to_datatypes env cls
      in
      Set.diff mixed right

  let are_disjoint set1 set2 =
    match Set.disjoint () set1 set2 with
    | Set.Sat -> true
    | _ -> false
end

type runtime_data_type = decl_ty * DataType.t

let data_type_from_hint (env : env) (hint : Aast.hint) : runtime_data_type =
  let decl_ty = Decl_hint.hint env.decl_env hint in
  let ((env, _), ty) =
    Typing_utils.localize_no_subst env ~ignore_errors:true decl_ty
  in
  (decl_ty, DataType.fromTy env ty)

let check_overlapping env (ty1, data_type1) (ty2, data_type2) =
  match DataType.Set.disjoint () data_type1 data_type2 with
  | DataType.Set.Sat -> None
  | DataType.Set.Unsat (tag, _) ->
    let tag_str = Tag.name tag in
    let why () =
      let why_ty ~f ty =
        let ty_str = Typing_print.full_strip_ns_decl env ty in
        Reason.to_string (f ty_str tag_str) (Typing_defs.get_reason ty)
      in
      let why1 =
        why_ty
          ty1
          ~f:
            (Printf.sprintf "This is the type `%s`, which includes `%s` values")
      in
      let why2 =
        why_ty
          ty2
          ~f:
            (Printf.sprintf
               "It overlaps with `%s`, which also includes `%s` values")
      in
      why1 @ why2
    in
    Some (tag_str, Lazy.from_fun why)

(**
 * Given the variants of a case type (encoded as a locl_ty) and another locl_ty [intersecting_ty]
 * produce a new locl_ty containing only the types in the variant that map to an intersecting
 * data type. For example:
 *  Given
 *   [variants] = int | vec<int> | Vector<int>
 *   [intersecting_ty] = Container<string>
 *
 *  This function will return the type `vec<int> | Vector<int>` because both `vec<int>` and
 * `Vector<int>` overlap with the tag associated with `Container<string>`.
 *
 * Note that this function only considers the data type associated to each type and not
 * the type itself. So even though `vec<int>` and `Container<string>` do not intersect at
 * the type level, they do intersect when considering only the runtime data types.
 *)
let filter_variants_using_datatype env reason variants intersecting_ty =
  let tags = DataType.fromTy env intersecting_ty in
  let tyl =
    List.filter
      ~f:(fun ty ->
        let variant_tags = DataType.fromTy env ty in
        not @@ DataType.are_disjoint variant_tags tags)
      variants
  in
  Typing_utils.union_list env reason tyl

(**
 * Look up case type via [name]. If the case type exist returns the list of
 * variant types. If the case type doesn't exist, returns [None].
*)
let get_variant_tys env name ty_args :
    Typing_env_types.env * locl_ty list option =
  match Env.get_typedef env name with
  | Some { td_type = variants; td_vis = Aast.CaseType; td_tparams; _ } ->
    let ((env, _ty_err_opt), variants) =
      Typing_utils.localize
        ~ety_env:
          {
            empty_expand_env with
            substs =
              (if List.is_empty ty_args then
                SMap.empty
              else
                Decl_subst.make_locl td_tparams ty_args);
          }
        env
        variants
    in
    let tyl =
      match get_node variants with
      | Tunion tyl -> tyl
      | _ -> [variants]
    in
    (env, Some tyl)
  | _ -> (env, None)
