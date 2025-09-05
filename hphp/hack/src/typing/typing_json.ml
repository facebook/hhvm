(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@alert "-dependencies"]
(* So we don't have to pass in a typing env in order
   * to print things.
*)

(*****************************************************************************)
(* Pretty printing of types *)
(*****************************************************************************)

open Hh_prelude
open Option.Monad_infix
open Typing_defs
open Typing_env_types
module SN = Naming_special_names
module Reason = Typing_reason
module Cls = Folded_class
module Nast = Aast

let show_supportdyn env =
  (not (TypecheckerOptions.everything_sdt env.genv.tcopt))
  || Typing_env_types.get_log_level env "show" >= 1

open Hh_json

let param_mode_to_string = function
  | FPnormal -> "normal"
  | FPinout -> "inout"

let string_to_param_mode = function
  | "normal" -> Some FPnormal
  | "inout" -> Some FPinout
  | _ -> None

let is_like ty =
  match get_node ty with
  | Tunion tyl -> List.exists tyl ~f:is_dynamic
  | _ -> false

let cstr_kind cstr_kind =
  let kind_str =
    match cstr_kind with
    | Ast_defs.Constraint_as -> "as"
    | Ast_defs.Constraint_super -> "super"
    | Ast_defs.Constraint_eq -> "eq"
  in
  ("kind", Hh_json.JSON_String kind_str)

let user_attribute { ua_name = (_, name); _ } =
  let open Hh_json in
  let fields = [("name", JSON_String name)] in
  JSON_Object fields

let rec from_type : env -> show_like_ty:bool -> locl_ty -> json =
 fun env ~show_like_ty ty ->
  (* Helpers to construct fields that appear in JSON rendering of type *)
  let obj x = JSON_Object x in
  let kind p k = [("src_pos", Pos_or_decl.json p); ("kind", JSON_String k)] in
  let args tys =
    [("args", JSON_Array (List.map tys ~f:(from_type env ~show_like_ty)))]
  in
  let optional_args tys =
    if List.is_empty tys then
      []
    else
      [
        ( "optional_args",
          JSON_Array (List.map tys ~f:(from_type env ~show_like_ty)) );
      ]
  in
  let variadic_arg ty =
    if is_nothing ty then
      []
    else
      [("variadic_arg", from_type env ~show_like_ty ty)]
  in
  let splat_arg ty = [("splat_arg", from_type env ~show_like_ty ty)] in
  let refs e =
    match e with
    | Exact -> []
    | Nonexact r when Class_refinement.is_empty r -> []
    | Nonexact { cr_consts } ->
      let ref_const (id, { rc_bound; rc_is_ctx }) =
        let is_ctx_json = ("is_ctx", JSON_Bool rc_is_ctx) in
        match rc_bound with
        | TRexact ty ->
          obj
            [
              ("type", JSON_String id);
              ("equal", from_type env ~show_like_ty ty);
              is_ctx_json;
            ]
        | TRloose { tr_lower; tr_upper } ->
          let ty_list tys =
            JSON_Array (List.map tys ~f:(from_type env ~show_like_ty))
          in
          obj
            [
              ("type", JSON_String id);
              ("lower", ty_list tr_lower);
              ("upper", ty_list tr_upper);
              is_ctx_json;
            ]
      in
      [("refs", JSON_Array (List.map (SMap.bindings cr_consts) ~f:ref_const))]
  in
  let typ ty = [("type", from_type env ~show_like_ty ty)] in
  let result ty = [("result", from_type env ~show_like_ty ty)] in
  let name x = [("name", JSON_String x)] in
  let optional x = [("optional", JSON_Bool x)] in
  let is_array x = [("is_array", JSON_Bool x)] in
  let shape_field_name_to_json shape_field =
    (* TODO: need to update userland tooling? *)
    match shape_field with
    | Typing_defs.TSFregex_group (_, s) -> Hh_json.JSON_Number s
    | Typing_defs.TSFlit_str (_, s) -> Hh_json.JSON_String s
    | Typing_defs.TSFclass_const ((_, s1), (_, s2)) ->
      Hh_json.JSON_Array [Hh_json.JSON_String s1; Hh_json.JSON_String s2]
  in
  let make_field (k, v) =
    obj
    @@ [("name", shape_field_name_to_json k)]
    @ optional v.sft_optional
    @ typ v.sft_ty
  in
  let fields fl = [("fields", JSON_Array (List.map fl ~f:make_field))] in
  let as_type ty = [("as", from_type env ~show_like_ty ty)] in
  match (get_pos ty, get_node ty) with
  | (_, Tvar n) ->
    let (_, ty) =
      Typing_inference_env.expand_type
        env.inference_env
        (mk (get_reason ty, Tvar n))
    in
    begin
      match (get_pos ty, get_node ty) with
      | (p, Tvar _) -> obj @@ kind p "var"
      | _ -> from_type env ~show_like_ty ty
    end
  | (p, Ttuple { t_required; t_extra = Textra { t_optional; t_variadic } }) ->
    obj
    @@ kind p "tuple"
    @ is_array false
    @ args t_required
    @ optional_args t_optional
    @ variadic_arg t_variadic
  | (p, Ttuple { t_required; t_extra = Tsplat ty }) ->
    obj @@ kind p "tuple" @ is_array false @ args t_required @ splat_arg ty
  | (p, Tany _) -> obj @@ kind p "any"
  | (p, Tnonnull) -> obj @@ kind p "nonnull"
  | (p, Tdynamic) -> obj @@ kind p "dynamic"
  | (p, Tgeneric s) -> obj @@ kind p "generic" @ name s
  | (_, Tnewtype (s, [ty], _))
    when String.equal s SN.Classes.cSupportDyn && not (show_supportdyn env) ->
    from_type env ~show_like_ty ty
  | (p, Tnewtype (s, _, ty))
    when Decl_provider.get_class env.decl_env.Decl_env.ctx s
         |> Decl_entry.to_option
         >>| Cls.enum_type
         |> Option.is_some ->
    obj @@ kind p "enum" @ name s @ as_type ty
  | (p, Tnewtype (s, tys, ty)) ->
    obj @@ kind p "newtype" @ name s @ args tys @ as_type ty
  | (p, Tdependent (DTexpr _, ty)) ->
    obj
    @@ kind p "path"
    @ [("type", obj @@ kind (get_pos ty) "expr")]
    @ as_type ty
  | (p, Toption ty) -> begin
    match get_node ty with
    | Tnonnull -> obj @@ kind p "mixed"
    | _ -> obj @@ kind p "nullable" @ args [ty]
  end
  | (p, Tprim tp) ->
    obj @@ kind p "primitive" @ name (Aast_defs.string_of_tprim tp)
  | (p, Tneg predicate) ->
    let rec tag_json tag =
      match tag with
      | BoolTag -> name "isbool"
      | IntTag -> name "isint"
      | StringTag -> name "isstring"
      | ArraykeyTag -> name "isarraykey"
      | FloatTag -> name "isfloat"
      | NumTag -> name "isnum"
      | ResourceTag -> name "isresource"
      | NullTag -> name "isnull"
      | ClassTag (s, args) ->
        let generic_json g =
          match g with
          | Filled ty ->
            obj
              [
                ("generic_kind", JSON_String "filled");
                ("type", from_type env ~show_like_ty ty);
              ]
          | Wildcard _ -> obj [("generic_kind", JSON_String "wildcard")]
        in
        let args_json = List.map args ~f:generic_json in
        name s @ [("args", JSON_Array args_json)]
    and predicate_json predicate =
      match snd predicate with
      | IsTag tag -> tag_json tag
      | IsTupleOf { tp_required } ->
        let predicates_json =
          List.map tp_required ~f:(fun p -> obj @@ predicate_json p)
        in
        name "istuple" @ [("args", JSON_Array predicates_json)]
      | IsShapeOf { sp_fields } ->
        name "isshape"
        @ [
            ( "fields",
              JSON_Array
                (List.map
                   ~f:(fun (field, { sfp_predicate }) ->
                     obj
                     @@ [
                          ("name", shape_field_name_to_json field);
                          ("predicate", obj @@ predicate_json sfp_predicate);
                        ])
                   (TShapeMap.bindings sp_fields)) );
          ]
      (* TODO: T196048813 optional, open, fuel? *)
    in
    obj @@ kind p "negation" @ predicate_json predicate
  | (p, Tclass ((_, cid), e, tys)) ->
    obj @@ kind p "class" @ name cid @ args tys @ refs e
  | (p, Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fl }) ->
    let fields_known = is_nothing shape_kind in
    obj
    @@ kind p "shape"
    @ is_array false
    @ [("fields_known", JSON_Bool fields_known)]
    @ fields (TShapeMap.bindings fl)
  | (p, Tunion []) -> obj @@ kind p "nothing"
  | (_, Tunion [ty]) -> from_type env ~show_like_ty ty
  | (p, Tunion tyl) ->
    if show_like_ty then
      obj @@ kind p "union" @ args tyl
    else begin
      match List.filter tyl ~f:(fun ty -> not (is_dynamic ty)) with
      | [ty] -> from_type env ~show_like_ty ty
      | _ -> obj @@ kind p "union" @ args tyl
    end
  | (p, Tintersection []) -> obj @@ kind p "mixed"
  | (_, Tintersection [ty]) -> from_type env ~show_like_ty ty
  | (p, Tintersection tyl) ->
    if show_like_ty then
      obj @@ kind p "intersection" @ args tyl
    else begin
      match List.find tyl ~f:is_like with
      | None -> obj @@ kind p "intersection" @ args tyl
      | Some ty -> from_type env ~show_like_ty ty
    end
  | (p, Tfun ft) ->
    let fun_kind p = kind p "function" in
    let callconv cc =
      [("callConvention", JSON_String (param_mode_to_string cc))]
    in
    let readonly_param ro =
      if ro then
        [("readonly", JSON_Bool true)]
      else
        []
    in
    let optional_param opt =
      if opt then
        [("optional", JSON_Bool true)]
      else
        []
    in
    let param fp =
      obj
      @@ callconv (get_fp_mode fp)
      @ readonly_param (get_fp_readonly fp)
      @ optional_param (get_fp_is_optional fp)
      @ typ fp.fp_type
    in
    let readonly_this ro =
      if ro then
        [("readonly_this", JSON_Bool true)]
      else
        []
    in
    let readonly_ret ro =
      if ro then
        [("readonly_return", JSON_Bool true)]
      else
        []
    in
    let params fps = [("params", JSON_Array (List.map fps ~f:param))] in

    (* Function type parameters shouldn't be declared with variance annotations *)
    let tparams ft_tparams =
      [
        ( "tparams",
          JSON_Array
            (List.map
               ft_tparams
               ~f:(fun
                    Typing_defs_core.
                      { tp_name; tp_constraints; tp_user_attributes; _ }
                  ->
                 obj
                   [
                     ("name", JSON_String (snd tp_name));
                     ( "constraints",
                       JSON_Array
                         (List.map tp_constraints ~f:(fun (c, ty) ->
                              obj (cstr_kind c :: typ ty))) );
                     ( "user_attributes",
                       JSON_Array
                         (List.map tp_user_attributes ~f:user_attribute) );
                   ])) );
      ]
    in
    let capability =
      match ft.ft_implicit_params.capability with
      | CapDefaults _ -> []
      | CapTy capty -> [("capability", from_type env ~show_like_ty capty)]
    in
    obj
    @@ fun_kind p
    @ readonly_this (get_ft_readonly_this ft)
    @ tparams ft.ft_tparams
    @ params ft.ft_params
    @ readonly_ret (get_ft_returns_readonly ft)
    @ result ft.ft_ret
    @ capability
  | (p, Tvec_or_dict (ty1, ty2)) ->
    obj @@ kind p "vec_or_dict" @ args [ty1; ty2]
  (* TODO akenn *)
  | (p, Taccess (ty, _id)) -> obj @@ kind p "type_constant" @ args [ty]
  | (p, Tlabel s) -> obj @@ kind p "label" @ name s
  | (p, Tclass_ptr ty) -> obj @@ kind p "class_ptr" @ typ ty

type deserialized_result = (locl_ty, deserialization_error) result

let wrap_json_accessor f x =
  match f x with
  | Ok value -> Ok value
  | Error access_failure ->
    Error
      (Deserialization_error
         (Hh_json.Access.access_failure_to_string access_failure))

let wrap_json_accessor_with_default ~default f x =
  match f x with
  | Ok value -> Ok value
  | Error (Hh_json.Access.Missing_key_error _) -> Ok default
  | Error access_failure ->
    Error
      (Deserialization_error
         (Hh_json.Access.access_failure_to_string access_failure))

let wrap_json_accessor_with_opt f x =
  match f x with
  | Ok value -> Ok (Some value)
  | Error (Hh_json.Access.Missing_key_error _) -> Ok None
  | Error access_failure ->
    Error
      (Deserialization_error
         (Hh_json.Access.access_failure_to_string access_failure))

let get_string x = wrap_json_accessor (Hh_json.Access.get_string x)

let get_bool x = wrap_json_accessor (Hh_json.Access.get_bool x)

let get_array x = wrap_json_accessor (Hh_json.Access.get_array x)

let get_array_opt x =
  wrap_json_accessor_with_default ~default:([], []) (Hh_json.Access.get_array x)

let get_val x = wrap_json_accessor (Hh_json.Access.get_val x)

let get_obj x = wrap_json_accessor (Hh_json.Access.get_obj x)

let get_obj_opt x = wrap_json_accessor_with_opt (Hh_json.Access.get_obj x)

let deserialization_error ~message ~keytrace =
  Error
    (Deserialization_error (message ^ Hh_json.Access.keytrace_to_string keytrace))

let not_supported ~message ~keytrace =
  Error (Not_supported (message ^ Hh_json.Access.keytrace_to_string keytrace))

let get_constraint_kind key (json, keytrace) =
  Result.(
    get_string key (json, keytrace) >>= fun (kind_str, keytrace) ->
    match kind_str with
    | "as" -> Ok Ast_defs.Constraint_as
    | "super" -> Ok Constraint_super
    | "eq" -> Ok Constraint_eq
    | _ -> deserialization_error ~message:"Expected constraint kind" ~keytrace)

let to_locl_ty ?(keytrace = []) (ctx : Provider_context.t) (json : Hh_json.json)
    : deserialized_result =
  let reason = Reason.none in
  let ty (ty : locl_phase ty_) : deserialized_result = Ok (mk (reason, ty)) in
  let rec aux (json : Hh_json.json) ~(keytrace : Hh_json.Access.keytrace) :
      deserialized_result =
    Result.Monad_infix.(
      get_string "kind" (json, keytrace) >>= fun (kind, kind_keytrace) ->
      match kind with
      | "this" ->
        not_supported ~message:"Cannot deserialize 'this' type." ~keytrace
      | "any" -> ty (Typing_defs.make_tany ())
      | "mixed" -> ty (Toption (mk (reason, Tnonnull)))
      | "nonnull" -> ty Tnonnull
      | "dynamic" -> ty Tdynamic
      | "label" ->
        get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
        ty (Tlabel name)
      | "generic" ->
        (*  (see Tgeneric case of serialization logic) *)
        get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
        ty (Tgeneric name)
      | "enum" ->
        get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
        aux_as json ~keytrace >>= fun as_ty -> ty (Tnewtype (name, [], as_ty))
      | "newtype" ->
        get_string "name" (json, keytrace) >>= fun (name, name_keytrace) ->
        begin
          match Decl_provider.get_typedef ctx name with
          | Decl_entry.Found _typedef ->
            (* We end up only needing the name of the typedef. *)
            Ok name
          | Decl_entry.DoesNotExist
          | Decl_entry.NotYetAvailable ->
            if String.equal name "HackSuggest" then
              not_supported
                ~message:"HackSuggest types for lambdas are not supported"
                ~keytrace
            else
              deserialization_error
                ~message:("Unknown newtype: " ^ name)
                ~keytrace:name_keytrace
        end
        >>= fun typedef_name ->
        get_array "args" (json, keytrace) >>= fun (args, args_keytrace) ->
        aux_args args ~keytrace:args_keytrace >>= fun args ->
        aux_as json ~keytrace >>= fun as_ty ->
        ty (Tnewtype (typedef_name, args, as_ty))
      | "path" ->
        get_obj "type" (json, keytrace) >>= fun (type_json, type_keytrace) ->
        get_string "kind" (type_json, type_keytrace)
        >>= fun (path_kind, path_kind_keytrace) ->
        get_array "path" (json, keytrace) >>= fun (ids_array, ids_keytrace) ->
        let ids =
          map_array ids_array ~keytrace:ids_keytrace ~f:(fun id_str ~keytrace ->
              match id_str with
              | JSON_String id -> Ok id
              | _ ->
                deserialization_error ~message:"Expected a string" ~keytrace)
        in
        ids >>= fun _ids ->
        begin
          match path_kind with
          | "expr" ->
            not_supported
              ~message:
                "Cannot deserialize path-dependent type involving an expression"
              ~keytrace
          | "this" ->
            aux_as json ~keytrace >>= fun _as_ty -> ty (Tgeneric "this")
          | path_kind ->
            deserialization_error
              ~message:("Unknown path kind: " ^ path_kind)
              ~keytrace:path_kind_keytrace
        end
      | "tuple" ->
        get_array "args" (json, keytrace) >>= fun (args, args_keytrace) ->
        aux_args args ~keytrace:args_keytrace >>= fun t_required ->
        get_obj_opt "splat_arg" (json, keytrace) >>= fun splat_arg_obj_opt ->
        begin
          match splat_arg_obj_opt with
          | Some (splat_arg_obj, keytrace) ->
            aux splat_arg_obj ~keytrace >>= fun t_splat ->
            ty (Ttuple { t_required; t_extra = Tsplat t_splat })
          | None ->
            get_array_opt "optional_args" (json, keytrace)
            >>= fun (optional_args, optional_args_keytrace) ->
            aux_args optional_args ~keytrace:optional_args_keytrace
            >>= fun t_optional ->
            get_obj_opt "variadic_arg" (json, keytrace) >>= fun popt ->
            aux_variadic_arg popt >>= fun t_variadic ->
            ty
              (Ttuple
                 { t_required; t_extra = Textra { t_optional; t_variadic } })
        end
      | "nullable" ->
        get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
        begin
          match args with
          | [nullable_ty] ->
            aux nullable_ty ~keytrace:("0" :: keytrace) >>= fun nullable_ty ->
            ty (Toption nullable_ty)
          | _ ->
            deserialization_error
              ~message:
                (Printf.sprintf
                   "Unsupported number of args for nullable type: %d"
                   (List.length args))
              ~keytrace
        end
      | "primitive" ->
        get_string "name" (json, keytrace) >>= fun (name, keytrace) ->
        begin
          match name with
          | "void" -> Ok Nast.Tvoid
          | "int" -> Ok Nast.Tint
          | "bool" -> Ok Nast.Tbool
          | "float" -> Ok Nast.Tfloat
          | "string" -> Ok Nast.Tstring
          | "resource" -> Ok Nast.Tresource
          | "num" -> Ok Nast.Tnum
          | "arraykey" -> Ok Nast.Tarraykey
          | "noreturn" -> Ok Nast.Tnoreturn
          | _ ->
            deserialization_error
              ~message:("Unknown primitive type: " ^ name)
              ~keytrace
        end
        >>= fun prim_ty -> ty (Tprim prim_ty)
      | "class" ->
        get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
        let class_pos =
          match Decl_provider.get_class ctx name with
          | Decl_entry.Found class_ty -> Cls.pos class_ty
          | Decl_entry.DoesNotExist
          | Decl_entry.NotYetAvailable ->
            (* Class may not exist (such as in non-strict modes). *)
            Pos_or_decl.none
        in
        get_array "args" (json, keytrace) >>= fun (args, _args_keytrace) ->
        aux_args args ~keytrace >>= fun tyl ->
        let refs =
          match get_array "refs" (json, keytrace) with
          | Ok (l, _) -> l
          | Error _ -> []
        in
        aux_refs refs ~keytrace >>= fun rs ->
        (* NB: "class" could have come from either a `Tapply` or a `Tclass`.
         * Right now, we always return a `Tclass`. *)
        ty (Tclass ((class_pos, name), Nonexact rs, tyl))
      | "shape" ->
        get_array "fields" (json, keytrace) >>= fun (fields, fields_keytrace) ->
        get_bool "is_array" (json, keytrace)
        >>= fun (is_array, _is_array_keytrace) ->
        let unserialize_field field_json ~keytrace :
            ( Typing_defs.tshape_field_name
              * locl_phase Typing_defs.shape_field_type,
              deserialization_error )
            result =
          get_val "name" (field_json, keytrace) >>= fun (name, name_keytrace) ->
          (* We don't need position information for shape field names. They're
           * only used for error messages and the like. *)
          let dummy_pos = Pos_or_decl.none in
          begin
            match name with
            | Hh_json.JSON_Number name ->
              Ok (Typing_defs.TSFregex_group (dummy_pos, name))
            | Hh_json.JSON_String name ->
              Ok (Typing_defs.TSFlit_str (dummy_pos, name))
            | Hh_json.JSON_Array
                [Hh_json.JSON_String name1; Hh_json.JSON_String name2] ->
              Ok
                (Typing_defs.TSFclass_const
                   ((dummy_pos, name1), (dummy_pos, name2)))
            | _ ->
              deserialization_error
                ~message:"Unexpected format for shape field name"
                ~keytrace:name_keytrace
          end
          >>= fun shape_field_name ->
          (* Optional field may be absent for shape-like arrays. *)
          begin
            match get_val "optional" (field_json, keytrace) with
            | Ok _ ->
              get_bool "optional" (field_json, keytrace)
              >>| fun (optional, _optional_keytrace) -> optional
            | Error _ -> Ok false
          end
          >>= fun optional ->
          get_obj "type" (field_json, keytrace)
          >>= fun (shape_type, shape_type_keytrace) ->
          aux shape_type ~keytrace:shape_type_keytrace
          >>= fun shape_field_type ->
          let shape_field_type =
            { sft_optional = optional; sft_ty = shape_field_type }
          in
          Ok (shape_field_name, shape_field_type)
        in
        map_array fields ~keytrace:fields_keytrace ~f:unserialize_field
        >>= fun fields ->
        if is_array then
          (* We don't have enough information to perfectly reconstruct shape-like
           * arrays. We're missing the keys in the shape map of the shape fields. *)
          not_supported
            ~message:"Cannot deserialize shape-like array type"
            ~keytrace
        else
          get_bool "fields_known" (json, keytrace)
          >>= fun (fields_known, _fields_known_keytrace) ->
          let shape_kind =
            if fields_known then
              Typing_make_type.nothing Reason.none
            else
              Typing_make_type.mixed Reason.none
          in
          let fields =
            List.fold fields ~init:TShapeMap.empty ~f:(fun shape_map (k, v) ->
                TShapeMap.add k v shape_map)
          in
          ty
            (Tshape
               {
                 s_origin = Missing_origin;
                 s_fields = fields;
                 s_unknown_value = shape_kind;
               })
      | "union" ->
        get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
        aux_args args ~keytrace >>= fun tyl -> ty (Tunion tyl)
      | "intersection" ->
        get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
        aux_args args ~keytrace >>= fun tyl -> ty (Tintersection tyl)
      | "function" ->
        get_array "tparams" (json, keytrace)
        >>= fun (tparams, tparams_keytrace) ->
        let tparams =
          map_array
            tparams
            ~keytrace:tparams_keytrace
            ~f:(fun param ~keytrace ->
              get_string "name" (param, keytrace)
              >>= fun (name, _name_keytrace) ->
              get_array "constraints" (json, keytrace)
              >>= fun (cstrs, keytrace) ->
              map_array cstrs ~keytrace ~f:aux_constraint
              >>= fun tp_constraints ->
              get_array "user_attributes" (json, keytrace)
              >>= fun (attrs, keytrace) ->
              map_array attrs ~keytrace ~f:aux_user_attribute
              >>= fun tp_user_attributes ->
              Ok
                {
                  tp_name = (Pos_or_decl.none, name);
                  tp_reified = Ast_defs.Erased;
                  tp_constraints;
                  tp_user_attributes;
                  tp_variance = Ast_defs.Invariant;
                })
        in
        tparams >>= fun ft_tparams ->
        get_array "params" (json, keytrace) >>= fun (params, params_keytrace) ->
        let params =
          map_array params ~keytrace:params_keytrace ~f:(fun param ~keytrace ->
              get_bool "optional" (param, keytrace)
              >>= fun (optional, _optional_keytrace) ->
              get_string "callConvention" (param, keytrace)
              >>= fun (callconv, callconv_keytrace) ->
              begin
                match string_to_param_mode callconv with
                | Some callconv -> Ok callconv
                | None ->
                  deserialization_error
                    ~message:("Unknown calling convention: " ^ callconv)
                    ~keytrace:callconv_keytrace
              end
              >>= fun callconv ->
              get_obj "type" (param, keytrace)
              >>= fun (param_type, param_type_keytrace) ->
              aux param_type ~keytrace:param_type_keytrace >>= fun param_type ->
              Ok
                {
                  fp_type = param_type;
                  fp_flags =
                    make_fp_flags
                      ~mode:callconv
                      ~accept_disposable:false
                      ~is_optional:optional
                      ~readonly:false
                      ~ignore_readonly_error:false
                      ~splat:false
                      ~named:false;
                  (* Dummy values: these aren't currently serialized. *)
                  fp_pos = Pos_or_decl.none;
                  fp_name = None;
                  fp_def_value = None;
                })
        in
        params >>= fun ft_params ->
        get_obj "result" (json, keytrace) >>= fun (result, result_keytrace) ->
        aux result ~keytrace:result_keytrace >>= fun ft_ret ->
        aux_capability json ~keytrace >>= fun capability ->
        aux_get_bool_with_default "readonly_this" false json ~keytrace
        >>= fun readonly_this ->
        aux_get_bool_with_default "readonly_return" false json ~keytrace
        >>= fun readonly_return ->
        aux_get_bool_with_default "instantiated" true json ~keytrace
        >>= fun ft_instantiated ->
        let funty =
          {
            ft_params;
            ft_implicit_params = { capability };
            ft_ret;
            ft_tparams;
            (* Dummy values: these aren't currently serialized. *)
            ft_where_constraints = [];
            ft_flags = Typing_defs_flags.Fun.default;
            ft_require_package = None;
            ft_instantiated;
          }
        in
        let funty = set_ft_returns_readonly funty readonly_return in
        let funty = set_ft_readonly_this funty readonly_this in
        ty (Tfun funty)
      | "nothing" -> ty (Tunion [])
      | _ ->
        deserialization_error
          ~message:
            (Printf.sprintf
               "Unknown or unsupported kind '%s' to convert to locl phase"
               kind)
          ~keytrace:kind_keytrace)
  and map_array :
      type a.
      Hh_json.json list ->
      f:
        (Hh_json.json ->
        keytrace:Hh_json.Access.keytrace ->
        (a, deserialization_error) result) ->
      keytrace:Hh_json.Access.keytrace ->
      (a list, deserialization_error) result =
   fun array ~f ~keytrace ->
    let array =
      List.mapi array ~f:(fun i elem ->
          f elem ~keytrace:(string_of_int i :: keytrace))
    in
    Result.all array
  and aux_args (args : Hh_json.json list) ~(keytrace : Hh_json.Access.keytrace)
      : (locl_ty list, deserialization_error) result =
    map_array args ~keytrace ~f:aux
  and aux_variadic_arg (arg : (Hh_json.json * Hh_json.Access.keytrace) option) :
      (locl_ty, deserialization_error) result =
    match arg with
    | None -> Ok (Typing_make_type.nothing Reason.none)
    | Some (ty, keytrace) -> aux ty ~keytrace
  and aux_refs (refs : Hh_json.json list) ~(keytrace : Hh_json.Access.keytrace)
      : (locl_class_refinement, deserialization_error) result =
    let of_refined_consts consts = { cr_consts = SMap.of_list consts } in
    Result.map ~f:of_refined_consts (map_array refs ~keytrace ~f:aux_ref)
  and aux_ref (json : Hh_json.json) ~(keytrace : Hh_json.Access.keytrace) :
      (string * locl_refined_const, deserialization_error) result =
    Result.Monad_infix.(
      get_bool "is_ctx" (json, keytrace) >>= fun (rc_is_ctx, _) ->
      get_string "type" (json, keytrace) >>= fun (id, _) ->
      match Hh_json.Access.get_obj "equal" (json, keytrace) with
      | Ok (ty_json, ty_keytrace) ->
        aux ty_json ~keytrace:ty_keytrace >>= fun ty ->
        Ok (id, { rc_bound = TRexact ty; rc_is_ctx })
      | Error (Hh_json.Access.Missing_key_error _) ->
        get_array "lower" (json, keytrace) >>= fun (los_json, los_keytrace) ->
        get_array "upper" (json, keytrace) >>= fun (ups_json, ups_keytrace) ->
        map_array los_json ~keytrace:los_keytrace ~f:aux >>= fun tr_lower ->
        map_array ups_json ~keytrace:ups_keytrace ~f:aux >>= fun tr_upper ->
        Ok (id, { rc_bound = TRloose { tr_lower; tr_upper }; rc_is_ctx })
      | Error _ as e -> wrap_json_accessor (fun _ -> e) ())
  and aux_as (json : Hh_json.json) ~(keytrace : Hh_json.Access.keytrace) :
      (locl_ty, deserialization_error) result =
    Result.Monad_infix.(
      (* as-constraint is optional, check to see if it exists. *)
      match Hh_json.Access.get_obj "as" (json, keytrace) with
      | Ok (as_json, as_keytrace) ->
        aux as_json ~keytrace:as_keytrace >>= fun as_ty -> Ok as_ty
      | Error (Hh_json.Access.Missing_key_error _) ->
        Ok (mk (Reason.none, Toption (mk (Reason.none, Tnonnull))))
      | Error access_failure ->
        deserialization_error
          ~message:
            ("Invalid as-constraint: "
            ^ Hh_json.Access.access_failure_to_string access_failure)
          ~keytrace)
  and aux_capability (json : Hh_json.json) ~(keytrace : Hh_json.Access.keytrace)
      : (locl_ty capability, deserialization_error) result =
    Result.Monad_infix.(
      match Hh_json.Access.get_obj "capability" (json, keytrace) with
      | Ok (cap_json, cap_keytrace) ->
        aux cap_json ~keytrace:cap_keytrace >>= fun cap_ty -> Ok (CapTy cap_ty)
      | Error (Hh_json.Access.Missing_key_error _) ->
        (* The "capability" key is omitted for CapDefaults during encoding *)
        Ok (CapDefaults Pos_or_decl.none)
      | Error access_failure ->
        deserialization_error
          ~message:
            ("Invalid capability: "
            ^ Hh_json.Access.access_failure_to_string access_failure)
          ~keytrace)
  and aux_get_bool_with_default
      key default (json : Hh_json.json) ~(keytrace : Hh_json.Access.keytrace) :
      (bool, deserialization_error) result =
    match Hh_json.Access.get_bool key (json, keytrace) with
    | Ok (value, _keytrace) -> Ok value
    | Error (Hh_json.Access.Missing_key_error _) -> Ok default
    | Error access_failure ->
      deserialization_error
        ~message:(Hh_json.Access.access_failure_to_string access_failure)
        ~keytrace
  and aux_constraint json ~keytrace =
    Result.(
      get_constraint_kind "kind" (json, keytrace) >>= fun kind ->
      get_val "type" (json, keytrace) >>= fun (ty_json, keytrace) ->
      aux ty_json ~keytrace >>= fun ty -> Ok (kind, ty))
  and aux_user_attribute json ~keytrace =
    Result.(
      get_string "name" (json, keytrace) >>= fun (name, _keytrace) ->
      Ok
        {
          ua_name = (Pos_or_decl.none, name);
          ua_params = [];
          ua_raw_val = None;
        })
  in
  aux json ~keytrace

let from_locl_ty env ?(show_like_ty = false) ty = from_type env ~show_like_ty ty
