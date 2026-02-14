(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Typing_env_types
module Cls = Folded_class
module Env = Typing_env
module SN = Naming_special_names

let strip_ns id =
  id |> Utils.strip_ns |> Hh_autoimport.strip_HH_namespace_if_autoimport

module Tag = struct
  include Typing_tag_defs

  type ctx = Typing_env_types.env

  let is_fresh_generic ty =
    match get_node ty with
    | Tgeneric name -> Env.is_fresh_generic_parameter name
    | _ -> false

  let generics_for_class_and_ty_opt_l env cls ty_opt_l =
    match Env.get_class env cls with
    | Decl_entry.Found class_info ->
      let tparams = Folded_class.tparams class_info in
      let rec zip tparams ty_opt_l =
        match (tparams, ty_opt_l) with
        | ([], _) -> []
        | ( { tp_reified = Aast.(SoftReified | Reified); _ } :: tparams,
            Some ty :: ty_opt_l )
          when not (is_fresh_generic ty) ->
          Reified ty :: zip tparams ty_opt_l
        | (_ :: tparams, _ :: (_ as ty_opt_l))
        | (_ :: tparams, ([] as ty_opt_l)) ->
          Erased :: zip tparams ty_opt_l
      in
      zip tparams ty_opt_l
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      List.map ty_opt_l ~f:(fun _ -> Erased)

  let generics_for_class_and_tyl env cls args =
    generics_for_class_and_ty_opt_l env cls @@ List.map args ~f:Option.some

  let generics_for_class_and_tag_generic_l env cls generic_l =
    generics_for_class_and_ty_opt_l env cls
    @@ List.map generic_l ~f:(function
           | Filled ty -> Some ty
           | Wildcard _ -> None)

  let describe_generics env generics =
    match generics with
    | _ :: _
      when List.exists generics ~f:(fun g -> not @@ equal_generic g Erased) ->
      "<"
      ^ (String.concat ~sep:", "
        @@ List.map generics ~f:(fun g ->
               match g with
               | Erased -> "_"
               | Reified ty ->
                 Typing_print.full_strip_ns ~hide_internals:true env ty))
      ^ ">"
    | _ -> ""

  let describe env = function
    | DictData -> "dicts"
    | VecData -> "vecs"
    | TupleData -> "tuples"
    | ShapeData -> "shapes"
    | KeysetData -> "keysets"
    | StringData -> "strings"
    | ResourceData -> "resources"
    | BoolData -> "booleans"
    | IntData -> "ints"
    | FloatData -> "floats"
    | NullData -> "the value null"
    | ObjectData -> "objects"
    | InstanceOf { name; kind = FinalClass generics } ->
      Printf.sprintf
        "instances of the final class %s%s"
        (strip_ns name)
        (describe_generics env generics)
    | InstanceOf { name; kind = Class generics } ->
      Printf.sprintf
        "instances of the class %s%s"
        (strip_ns name)
        (describe_generics env generics)
    | InstanceOf { name; kind = Interface } ->
      Printf.sprintf "instances of the interface %s" @@ strip_ns name
    | LabelData -> "enum class labels"
    | BuiltInData -> "built-in values"

  (* Given [(T1a, T1b), (T2a, T2b)...],
     what is the relationship between Foo<T1a, T2a, ...> and Foo<T1b, T2b, ...>
  *)
  let rec relation_generics env generic_pairs =
    match generic_pairs with
    | [] -> SetRelation.equivalent
    | (Erased, Erased) :: generic_pairs -> relation_generics env generic_pairs
    | (_, Erased) :: generic_pairs ->
      SetRelation.(inter subset (relation_generics env generic_pairs))
    | (Erased, Reified _) :: _ -> SetRelation.none
    | (Reified left, Reified right) :: generic_pairs ->
      let is_sub_left_right =
        Typing_utils.is_sub_type_opt_ignore_generic_params env left right
      in
      let is_sub_right_left =
        Typing_utils.is_sub_type_opt_ignore_generic_params env right left
      in
      begin
        match (is_sub_left_right, is_sub_right_left) with
        | (Some true, Some true) -> relation_generics env generic_pairs
        | (Some false, _)
        | (_, Some false) ->
          SetRelation.disjoint
        | _ -> SetRelation.none
      end

  let relation tag1 ~ctx:env tag2 =
    match (tag1, tag2) with
    (* Built-in data can be any value made exclusively by the runtime.
       It includes some tags that aren't able to be tested for within the surface language.
       Since this can include multiple data types at runtime, we should consider their,
       relationship to each other to be unknown *)
    | (BuiltInData, BuiltInData)
    (* Built-in data includes objects that are used as part of certain features that aren't
       exposed to the surface language. An example of this is MethCallerHelper, which is a
       the instance produce for calls to meth_caller(). This means there is some overlap
       between the set of values that are built-in and objects *)
    | (BuiltInData, ObjectData)
    | (ObjectData, BuiltInData) ->
      SetRelation.none
    (* Shapes are represented imprecisely so do not consider them as equal *)
    | (ShapeData, ShapeData)
    | (DictData, ShapeData)
    | (ShapeData, DictData) ->
      SetRelation.none
    (* Tuples are represented imprecisely so do not consider them as equal *)
    | (TupleData, TupleData)
    | (VecData, TupleData)
    | (TupleData, VecData) ->
      SetRelation.none
    | (tag1, tag2) when equal tag1 tag2 -> SetRelation.equivalent
    | (ObjectData, InstanceOf _) -> SetRelation.superset
    | (InstanceOf _, ObjectData) -> SetRelation.subset
    | ( InstanceOf { name = cls1; kind = kind1 },
        InstanceOf { name = cls2; kind = kind2 } ) ->
      let open Option.Let_syntax in
      let is_instance_of sub sup =
        let* cls = Decl_entry.to_option (Env.get_class env sub) in
        return @@ Cls.has_ancestor cls sup
      in
      let is_instance_of_closure cls = String.equal cls SN.Classes.cClosure in

      Option.value ~default:SetRelation.none
      @@ let* cls1_instance_of_cls2 = is_instance_of cls1 cls2 in
         if cls1_instance_of_cls2 then
           if has_reified kind2 then
             (* TODO(T221435654)
                if cls2 has reified generics and cls1 inherits cls2 but
                cls1 != cls2, we would need to know *how* cls1 inherits from
                cls2, i.e. what are the generics provided to cls2 in the
                inheritting declaration(s) *)
             return SetRelation.none
           else
             return SetRelation.subset
         else
           let* cls2_instance_of_cls1 = is_instance_of cls2 cls1 in
           if cls2_instance_of_cls1 then
             if has_reified kind1 then
               (* TODO(T221435654) see comment above *)
               return SetRelation.none
             else
               return SetRelation.superset
           else if String.equal cls1 cls2 then
             return
             @@ relation_generics env
             @@ zip_generics
                  (get_generics_from_kind kind1)
                  (get_generics_from_kind kind2)
           else (
             match (kind1, kind2) with
             | (FinalClass _, _)
             | (_, FinalClass _) ->
               return SetRelation.disjoint
             | (Interface, _)
             | (_, Interface) ->
               (* Closures cannot be implemented in user-land so we assume
                  it is disjoint from any user specified interface *)
               if is_instance_of_closure cls1 || is_instance_of_closure cls2
               then
                 return SetRelation.disjoint
               else
                 return SetRelation.none
             | (Class _, Class _) -> return SetRelation.disjoint
           )
    | _ -> SetRelation.disjoint
end

(** Tracks the reason why a particular tag is assumed to be a part of the
    data type of a type. Tracking is optional, depending on if an origin
    type is provided or not.
*)
module DataTypeReason = struct
  (** More information about where a particular tag comes from.
    For example, for tag DictData, subreasons might be Shapes or NoSubreason *)
  type subreason =
    | NoSubreason
    | Shapes
    | Tuples
    | Nullable
    | Nums
    | Arraykeys
    | SpecialInterface of string
    | Cyclic

  type trail_kind =
    | VariantOfCaseType of string
    | GenericUpperbound of string
    | UpperboundOfNewType of string
    | UpperboundOfEnum of string
    | SealedInterface of string
    | Requirement of string
    | ExpansionOfTypeConstant of {
        root_ty: locl_ty;
        name: string;
      }

  type trail = {
    instances: (trail_kind * Reason.t) list;
    expansions: Type_expansions.t;
  }

  type t = subreason * trail

  let append ~(trail : trail) ~(reason : Reason.t) (kind : trail_kind) :
      (trail, _) result =
    match kind with
    | VariantOfCaseType name
    | GenericUpperbound name
    | UpperboundOfNewType name
    | UpperboundOfEnum name
    | SealedInterface name
    | Requirement name ->
      Type_expansions.add_and_check_cycles
        trail.expansions
        {
          Type_expansions.name = Type_expansions.Expandable.Type_alias name;
          use_pos = Reason.to_pos reason;
          def_pos = None;
        }
      |> Result.map ~f:(fun expansions ->
             { instances = (kind, reason) :: trail.instances; expansions })
      |> Result.map_error ~f:(fun _cycle -> trail)
    | ExpansionOfTypeConstant _ ->
      Ok { trail with instances = (kind, reason) :: trail.instances }

  let case_type ~trail reason name =
    append ~trail ~reason @@ VariantOfCaseType name

  let generic ~trail reason name =
    append ~trail ~reason @@ GenericUpperbound name

  let newtype ~trail reason name =
    append ~trail ~reason @@ UpperboundOfNewType name

  let enum ~trail reason name = append ~trail ~reason @@ UpperboundOfEnum name

  let type_constant ~trail reason root_ty name =
    append ~trail ~reason @@ ExpansionOfTypeConstant { root_ty; name }

  let sealed_interface ~trail reason name =
    append ~trail ~reason @@ SealedInterface name

  let requirement ~trail reason name = append ~trail ~reason @@ Requirement name

  let make subreason trail : t = (subreason, trail)

  let make_trail : trail =
    { instances = []; expansions = Type_expansions.empty }
end

module TagWithReason = struct
  type ctx = Tag.ctx

  type t = {
    subreason: DataTypeReason.subreason;
    trail: DataTypeReason.trail;
    tag: Tag.t;
  }

  let relation { tag = tag1; _ } ~ctx { tag = tag2; _ } =
    Tag.relation tag1 ~ctx tag2

  let make (subreason, trail) tag = { subreason; trail; tag }

  let to_message env ~f (origin, { subreason; trail; tag }) =
    let open DataTypeReason in
    let ty_str =
      Typing_print.full_strip_ns_decl ~verbose_fun:false env origin
    in
    let pos = Reason.to_pos (get_reason origin) in
    let prefix = f ty_str in
    let tag_str = Tag.describe env tag in
    let subreason_str =
      match subreason with
      | NoSubreason -> ""
      | Shapes -> " because shapes are dicts at runtime"
      | Tuples -> " because tuples are vecs at runtime"
      | Nullable -> " because it is a nullable type"
      | Nums -> " because nums are ints or floats"
      | Arraykeys -> " because arraykeys are ints or strings"
      | Cyclic -> " because it is a cyclic type"
      | SpecialInterface name ->
        Printf.sprintf
          " because `%s` is a special interface that includes non-object values"
        @@ strip_ns name
    in
    let msg = prefix ^ Markdown_lite.md_bold tag_str ^ subreason_str in

    let trail_kind_msg = function
      | VariantOfCaseType name ->
        Printf.sprintf "  via this variant of the case type `%s`"
        @@ strip_ns name
      | GenericUpperbound name ->
        Printf.sprintf "  via this constraint on the generic `%s`" @@ name
      | UpperboundOfNewType name ->
        Printf.sprintf "  via the constraint on the newtype `%s`"
        @@ strip_ns name
      | UpperboundOfEnum name ->
        Printf.sprintf "  via the constraint on the enum `%s`" @@ strip_ns name
      | SealedInterface name ->
        Printf.sprintf "  via the sealed interface `%s`" @@ strip_ns name
      | Requirement name ->
        Printf.sprintf "  via this requirement on `%s`" @@ strip_ns name
      | ExpansionOfTypeConstant { root_ty; name } ->
        let name =
          Printf.sprintf
            "%s::%s"
            (Typing_print.full_strip_ns ~hide_internals:true env root_ty)
            name
        in
        Printf.sprintf "  via the expansion of the type constant `%s`" @@ name
    in
    let trail_result =
      List.fold trail.instances ~init:[] ~f:(fun acc (kind, reason) ->
          Reason.to_string (trail_kind_msg kind) reason @ acc)
    in
    (pos, msg) :: trail_result
end

module type SET = sig
  type t

  val empty : t

  val singleton : reason:DataTypeReason.t -> Typing_tag_defs.t -> t

  val of_list : reason:DataTypeReason.t -> Typing_tag_defs.t list -> t

  val union : t -> t -> t

  val inter : t -> t -> t

  val diff : t -> t -> t

  val are_disjoint : Typing_env_types.env -> t -> t -> bool
end

module ApproxTagSet = struct
  include ApproxSet.Make (TagWithReason)

  let singleton ~reason tag = singleton @@ TagWithReason.make reason tag

  let of_list ~reason tags =
    List.map ~f:(TagWithReason.make reason) tags |> of_list
end

module Make (Set : SET) = struct
  type t = Set.t

  (** `mixed` should cover all possible data types.
      Update [Tag.all_tags] if additional tags are added to [Tag.t] *)
  let mixed ~reason = Set.of_list ~reason Tag.all_tags

  (** Check [trail] for a cycle. If [trail] is an [Error], then
    a cycle has been detected and this will produce an error and return
    the result of [default]. If not, it will just call [f env trail] *)
  let cycle_handler
      ~(env : env)
      ~(f : env -> DataTypeReason.trail -> 'res)
      ~(default : reason:DataTypeReason.t -> 'res)
      ~(trail : (DataTypeReason.trail, DataTypeReason.trail) result) : _ =
    match trail with
    | Result.Ok trail -> f env trail
    | Result.Error trail -> default ~reason:DataTypeReason.(make Cyclic trail)

  let prim_to_datatypes ~trail (prim : Aast.tprim) : t =
    let open Tag in
    let open Aast in
    let reason = DataTypeReason.(make NoSubreason trail) in
    match prim with
    | Tint -> Set.singleton ~reason IntData
    | Tnull -> Set.singleton ~reason NullData
    | Tvoid -> Set.singleton ~reason NullData
    | Tbool -> Set.singleton ~reason BoolData
    | Tfloat -> Set.singleton ~reason FloatData
    | Tresource -> Set.singleton ~reason ResourceData
    | Tnum ->
      Set.of_list ~reason:DataTypeReason.(make Nums trail) [IntData; FloatData]
    | Tarraykey ->
      Set.of_list
        ~reason:DataTypeReason.(make Arraykeys trail)
        [IntData; StringData]
    | Tnoreturn -> Set.empty

  (** There are three ways values that are compatible with a function type can
      be introduce:
        1. Via a lambda expression (i.e. `() ==> {}` or `function () {}`)
        2. Via a meth_caller() expression
        3. Via a function reference expression (i.e. `f<>` or `Cls:meth<>`)
      Each can produce values of the following kind:
        1. Lambdas will be an instance of the special `Closure` class
        2. meth_caller() will be an instance of MethCallerHelper
        3. Function references have a dedicated datatype at runtime
      Out of these three possible kinds of value only `Closure` is denotable
      in the surface language, so the remaining two we represent as
      [Tag.BuiltinData]. *)
  let fun_to_datatypes ~trail : t =
    let open Tag in
    Set.of_list
      ~reason:DataTypeReason.(make NoSubreason trail)
      [BuiltInData; InstanceOf { name = SN.Classes.cClosure; kind = Class [] }]

  let nonnull_to_datatypes ~trail : t =
    Set.of_list
      ~reason:DataTypeReason.(make NoSubreason trail)
      Tag.all_nonnull_tags

  let tuple_to_datatypes ~trail : t =
    Set.singleton ~reason:DataTypeReason.(make Tuples trail) Tag.TupleData

  let shape_to_datatypes ~trail : t =
    Set.singleton ~reason:DataTypeReason.(make Shapes trail) Tag.ShapeData

  let label_to_datatypes ~trail : t =
    Set.singleton ~reason:DataTypeReason.(make NoSubreason trail) Tag.LabelData

  module Class : sig
    val to_datatypes :
      safe_for_are_disjoint:bool ->
      trail:DataTypeReason.trail ->
      env ->
      string ->
      Tag.generic list ->
      env * t
  end = struct
    (* Set of interfaces that contain non-object members *)
    let special_interfaces =
      SSet.of_list
        [
          SN.Classes.cStringish;
          SN.Classes.cXHPChild;
          SN.Classes.cString;
          SN.Collections.cTraversable;
          SN.Collections.cKeyedTraversable;
          SN.Collections.cContainer;
          SN.Collections.cKeyedContainer;
        ]

    (** The [special_interfaces] contain additional data type tags beside
     [ObjectData]. These special interfaces are handled in [Typing_subtype].
     To avoid duplicating the logic, we check if a certain tag should be included
     by doing a subtype test against the interface type. If the type
     corresponding to a tag is a subtype then it will be included in the list.
    *)
    let special_interface_to_datatypes env (interface : string) :
        env * Tag.t list =
      let open Tag in
      let open Typing_make_type in
      let r = Typing_reason.none in
      let tag_to_type =
        [
          (DictData, dict r (nothing r) (nothing r));
          (VecData, vec r (nothing r));
          (KeysetData, keyset r (nothing r));
          (StringData, string r);
          (ResourceData, resource r);
          (BoolData, bool r);
          (IntData, int r);
          (FloatData, float r);
          (NullData, null r);
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
        | name when String.equal name SN.Classes.cString ->
          (env, Typing_make_type.string Reason.none)
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
      (env, tags)

    let special_interface_cache : Tag.t list String.Table.t =
      String.Table.create ()

    let rec to_datatypes
        ~(safe_for_are_disjoint : bool)
        ~(trail : DataTypeReason.trail)
        (env : env)
        (cls : string)
        (generics : Tag.generic list) : t =
      let open Tag in
      let cycle_handler f = cycle_handler ~env ~f in
      let reason = DataTypeReason.(make NoSubreason trail) in
      let handle_sealed_whitelist ~trail ~name ~pos ~default whitelist =
        let trail =
          DataTypeReason.sealed_interface
            ~trail
            (Reason.witness_from_decl pos)
            name
        in
        SSet.fold
          (fun whitelist_cls acc ->
            cycle_handler ~trail ~default @@ fun env trail ->
            Set.union acc
            @@ to_datatypes
                 ~safe_for_are_disjoint
                 ~trail
                 env
                 whitelist_cls
                 (* You cannot provide generics when sealing *)
                 [])
          whitelist
          Set.empty
        |> Set.inter (default ~reason)
      in
      match cls with
      | cls when String.equal cls SN.Classes.cString ->
        Set.singleton ~reason StringData
      | cls when String.equal cls SN.Collections.cDict ->
        Set.singleton ~reason DictData
      | cls when String.equal cls SN.Collections.cKeyset ->
        Set.singleton ~reason KeysetData
      | cls when String.equal cls SN.Collections.cVec ->
        Set.singleton ~reason VecData
      | cls when String.equal cls SN.Collections.cAnyArray ->
        Set.of_list ~reason [DictData; KeysetData; VecData]
      | name ->
        Env.get_class env name
        |> Decl_entry.map_or
             ~default:(Set.singleton ~reason ObjectData)
             ~f:(fun cls ->
               let open Ast_defs in
               match Cls.kind cls with
               | Cclass _ when Cls.final cls ->
                 Set.singleton ~reason
                 @@ InstanceOf { name; kind = FinalClass generics }
               | Cclass _ ->
                 let default ~reason =
                   Set.singleton ~reason
                   @@ InstanceOf { name; kind = Class generics }
                 in
                 (match Cls.sealed_whitelist cls with
                 | None -> default ~reason
                 | Some whitelist ->
                   handle_sealed_whitelist
                     ~trail
                     ~name
                     ~pos:(Cls.pos cls)
                     ~default
                     whitelist)
               | Ctrait
               | Cinterface -> begin
                 let default ~reason =
                   if is_c_trait (Cls.kind cls) then
                     Set.singleton ~reason ObjectData
                   else
                     Set.singleton ~reason
                     @@ InstanceOf { name; kind = Interface }
                 in
                 match Cls.sealed_whitelist cls with
                 | None ->
                   let reqs = Cls.all_ancestor_reqs cls in
                   if safe_for_are_disjoint || List.is_empty reqs then
                     default ~reason
                   else
                     List.fold
                       ~init:(default ~reason)
                       ~f:(fun acc (_, required_ty) ->
                         match
                           Typing_utils.try_unwrap_class_type required_ty
                         with
                         | None -> acc
                         | Some (r, (_p, req_cls), _paraml) ->
                           let trail =
                             DataTypeReason.requirement
                               ~trail
                               (Reason.localize r)
                               name
                           in
                           cycle_handler ~trail ~default @@ fun env trail ->
                           Set.inter acc
                           @@ to_datatypes
                                ~safe_for_are_disjoint
                                ~trail
                                env
                                req_cls
                                (* TODO(T221435654)
                                   you can require a specific generic class,
                                   but for now, [] (all Erased) is sound but
                                   incomplete *)
                                [])
                       reqs
                 | Some whitelist ->
                   handle_sealed_whitelist
                     ~trail
                     ~name
                     ~pos:(Cls.pos cls)
                     ~default
                     whitelist
               end
               | Cenum
               | Cenum_class _ ->
                 Set.singleton ~reason ObjectData)

    let to_datatypes
        ~safe_for_are_disjoint
        ~trail
        (env : env)
        (cls : string)
        (args : Tag.generic list) : env * t =
      if SSet.mem cls special_interfaces then
        let (env, tags) =
          match Hashtbl.find special_interface_cache cls with
          | Some tags -> (env, tags)
          | None ->
            let (env, tags) = special_interface_to_datatypes env cls in
            let (_ : [ `Duplicate | `Ok ]) =
              Hashtbl.add special_interface_cache ~key:cls ~data:tags
            in
            (env, tags)
        in
        let set1 =
          Set.of_list
            ~reason:DataTypeReason.(make (SpecialInterface cls) trail)
            tags
        in
        let set2 = to_datatypes ~safe_for_are_disjoint ~trail env cls args in
        (env, Set.union set1 set2)
      else
        (env, to_datatypes ~safe_for_are_disjoint ~trail env cls args)
  end

  let rec fromPredicate
      ~safe_for_are_disjoint ~trail (env : env) (predicate : type_predicate) :
      env * t =
    let open Tag in
    let reason = DataTypeReason.(make NoSubreason trail) in
    let from_tag tag =
      match tag with
      | BoolTag -> (env, Set.singleton ~reason BoolData)
      | IntTag -> (env, Set.singleton ~reason IntData)
      | ArraykeyTag -> (env, Set.of_list ~reason [IntData; StringData])
      | FloatTag -> (env, Set.singleton ~reason FloatData)
      | NumTag -> (env, Set.of_list ~reason [IntData; FloatData])
      | ResourceTag -> (env, Set.singleton ~reason ResourceData)
      | NullTag -> (env, Set.singleton ~reason NullData)
      | ClassTag (id, _)
        when String.equal Naming_special_names.Classes.cString id ->
        (env, Set.singleton ~reason StringData)
      | ClassTag (id, args) ->
        let generics = Tag.generics_for_class_and_tag_generic_l env id args in
        Class.to_datatypes ~safe_for_are_disjoint ~trail env id generics
    in
    match snd predicate with
    | IsTag tag -> from_tag tag
    | IsTupleOf _ -> (env, Set.singleton ~reason VecData)
    | IsShapeOf _ -> (env, Set.singleton ~reason DictData)
    | IsUnionOf predicates ->
      let (env, sets) =
        List.fold_map predicates ~init:env ~f:(fun env pred ->
            fromPredicate ~safe_for_are_disjoint ~trail env pred)
      in
      (env, List.fold sets ~init:Set.empty ~f:Set.union)

  type context = {
    safe_for_are_disjoint: bool;
    trail: DataTypeReason.trail;
  }

  let rec fromTy (ctx : context) (env : env) (ty : locl_ty) : env * t =
    let (_, env, ty) = Typing_utils.strip_supportdyn env ty in
    let (env, ty) = Env.expand_type env ty in
    let trail = ctx.trail in
    let reason = DataTypeReason.(make NoSubreason trail) in
    let cycle_handler ty =
      cycle_handler
        ~default:(fun ~reason -> (env, mixed ~reason))
        ~f:(fun env trail -> fromTy { ctx with trail } env ty)
    in
    match get_node ty with
    | Tprim prim -> (env, prim_to_datatypes ~trail prim)
    | Tnonnull -> (env, nonnull_to_datatypes ~trail)
    | Tdynamic -> (env, mixed ~reason)
    | Tany _ -> (env, mixed ~reason)
    | Toption ty ->
      let (env, data_types) = fromTy ctx env ty in
      let data_types =
        Set.union
          data_types
          (Set.singleton
             ~reason:DataTypeReason.(make Nullable trail)
             Tag.NullData)
      in
      (env, data_types)
    (* For now say it has the same tags as `nonnull`.
     * We should be able to be more precise, but need to
     * validate what are all the data types that are valid callables *)
    | Tfun _ -> (env, fun_to_datatypes ~trail)
    | Ttuple _ -> (env, tuple_to_datatypes ~trail)
    | Tshape _ -> (env, shape_to_datatypes ~trail)
    | Tlabel _ -> (env, label_to_datatypes ~trail)
    | Tvar _ -> (env, mixed ~reason)
    | Tgeneric name ->
      let upper_bounds = Env.get_upper_bounds env name |> Typing_set.elements in
      let (env, sets) =
        List.fold_map
          ~init:env
          ~f:(fun env upper_bound_ty ->
            let trail =
              DataTypeReason.generic ~trail (get_reason upper_bound_ty) name
            in
            cycle_handler ~env ~trail upper_bound_ty)
          upper_bounds
      in
      let result_opt = List.reduce ~f:Set.inter sets in
      let res =
        Option.value_or_thunk result_opt ~default:(fun () -> mixed ~reason)
      in
      (env, res)
    | Tunion tyl ->
      List.fold tyl ~init:(env, Set.empty) ~f:(fun (env, acc) ty ->
          let (env, dty) = fromTy ctx env ty in
          let dty = Set.union acc dty in
          (env, dty))
    | Tintersection tyl ->
      let (env, sets) =
        List.fold_map ~init:env ~f:(fun env ty -> fromTy ctx env ty) tyl
      in
      let result_opt = List.reduce ~f:Set.inter sets in
      let res =
        Option.value_or_thunk result_opt ~default:(fun () -> mixed ~reason)
      in
      (env, res)
    | Tvec_or_dict _ -> (env, Set.of_list ~reason [Tag.VecData; Tag.DictData])
    | Taccess (root_ty, id) ->
      let ety_env = empty_expand_env in
      let ((env, _, _cycles), ty) =
        Typing_utils.expand_typeconst
          ety_env
          env
          root_ty
          id
          ~allow_abstract_tconst:true
      in
      let trail =
        DataTypeReason.type_constant ~trail (get_reason ty) root_ty (snd id)
      in
      cycle_handler ~env ~trail ty
    | Tdependent (_, ty) -> fromTy ctx env ty
    | Tnewtype (name, _, _)
      when String.equal name Naming_special_names.Classes.cEnumClassLabel ->
      (env, label_to_datatypes ~trail)
    | Tnewtype (name, ty_args, _) ->
      let (env, as_ty) =
        Typing_utils.get_newtype_super env (get_reason ty) name ty_args
      in
      begin
        (* A Tnewtype is either an opaque alias (defined with keyword `newtype`)
         * or a case type.
         * For opaque type aliases, we look at the upper bound, but for case types,
         * we expand. *)
        match Env.get_typedef env name with
        | Decl_entry.Found
            { td_type_assignment = CaseType (variant, variants); td_tparams; _ }
          ->
          (* Here we should expand the case type instead of looking at the upper bound.
           * If we do not expand, then we will over approximate the datatype.
           * Consider:
           *
           *  case type Type1 = bool | int;
           *  case type Type2 = Type1 | string;
           *
           * If we do not expand we will reject the definition of `Type2`
           * because we will believe the datatype for `Type1` contains `string`.
           * By expanding we can allow this definition. *)
          let localize env ty =
            Typing_utils.localize
              ~ety_env:
                (* The this_ty does not need to be set because case types cannot
                 * appear within classes thus cannot use the this type.
                 * If we ever change that this could need to be changed *)
                {
                  empty_expand_env with
                  substs = Decl_subst.make_locl td_tparams ty_args;
                }
              env
              ty
          in
          (* Ignore case type where clauses for now.
           * This will over-approximate the datatype because it will include all
           * the variants' types regardless of satisfaction of the where clause.
           * Over-approximation is safe, but incomplete: e.g. the following is OK
           * but results in an error if where clauses are ignored.
           *   case type Type1<T> = bool where T as arraykey | int;
           *   case type Type2 = bool | Type1<float>;
           * We can revisit should there be a use case for this.
           *)
          let tyl = List.map (variant :: variants) ~f:fst in
          List.fold tyl ~init:(env, Set.empty) ~f:(fun (env, acc) variant ->
              let ((env, _ty_err_opt), variant) = localize env variant in
              let trail =
                DataTypeReason.case_type ~trail (get_reason variant) name
              in
              let (env, dty) = cycle_handler ~env ~trail variant in
              let data_types = Set.union acc dty in
              (env, data_types))
        | _ ->
          let trail_f =
            if Env.is_enum env name || Env.is_enum_class env name then
              DataTypeReason.enum ~trail
            else
              DataTypeReason.newtype ~trail
          in
          let trail = trail_f (get_reason as_ty) name in
          cycle_handler ~env ~trail as_ty
      end
    | Tclass ((_, cls), _, _) when String.equal cls SN.Classes.cString ->
      let reason = DataTypeReason.(make NoSubreason trail) in
      (env, Set.singleton ~reason StringData)
    | Tclass ((_, cls), _, args) ->
      let generics = Tag.generics_for_class_and_tyl env cls args in
      Class.to_datatypes
        ~safe_for_are_disjoint:ctx.safe_for_are_disjoint
        ~trail
        env
        cls
        generics
    | Tneg predicate ->
      let (env, right) =
        fromPredicate
          ~safe_for_are_disjoint:ctx.safe_for_are_disjoint
          ~trail
          env
          predicate
      in
      (env, Set.diff (mixed ~reason) right)
    | Tclass_ptr _ ->
      (* TODO(T199606542) This likely needs a new DataType to represent
       * class pointers but should also take care to model the tri-state
       * while the types are still on logging enforcement *)
      (env, mixed ~reason)

  let fromHint ~safe_for_are_disjoint env hint =
    let decl_ty = Decl_hint.hint env.decl_env hint in
    let ((env, _), ty) =
      Typing_utils.localize_no_subst env ~ignore_errors:true decl_ty
    in
    let trail = DataTypeReason.make_trail in
    let (env, dt) = fromTy { safe_for_are_disjoint; trail } env ty in
    (env, (decl_ty, dt))

  let fromTy ~safe_for_are_disjoint env ty : env * t =
    let trail = DataTypeReason.make_trail in
    fromTy { safe_for_are_disjoint; trail } env ty
end

module DataType = struct
  module Set = ApproxTagSet
  include Make (ApproxTagSet)

  let trail = DataTypeReason.make_trail

  let prim_to_datatypes prim = prim_to_datatypes ~trail prim

  let fun_to_datatypes = fun_to_datatypes ~trail

  let nonnull_to_datatypes = nonnull_to_datatypes ~trail

  let tuple_to_datatypes = tuple_to_datatypes ~trail

  let shape_to_datatypes = shape_to_datatypes ~trail

  let label_to_datatypes = label_to_datatypes ~trail

  let mixed = mixed ~reason:DataTypeReason.(make NoSubreason trail)

  module Class = struct
    let to_datatypes ~safe_for_are_disjoint env cls generics =
      Class.to_datatypes ~safe_for_are_disjoint ~trail env cls generics
  end
end
