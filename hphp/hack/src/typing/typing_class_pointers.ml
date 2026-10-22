(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_defs
open Aast_defs

let error_at_cls_const_expr env pos cls_name =
  Typing_error_utils.add_typing_error
    ~env
    Typing_error.(primary @@ Primary.Class_const_to_string { pos; cls_name })

let error_at_cls_ptr_type env pos ty =
  Typing_error_utils.add_typing_error
    ~env
    Typing_error.(primary @@ Primary.Class_pointer_to_string { pos; ty })

let warning_at_classname_type env pos cls_name ty_pos =
  Typing_warning_utils.add
    env
    Typing_warning.
      ( pos,
        String_to_class_pointer,
        String_to_class_pointer.{ cls_name; ty_pos } )

let error_at_classname_type env pos cls_name ty_pos =
  Typing_error_utils.add_typing_error
    ~env
    Typing_error.(
      primary @@ Primary.String_to_class_pointer { pos; cls_name; ty_pos })

let string_of_class_id_ = function
  | CIparent -> "parent"
  | CIself -> "self"
  | CIstatic -> "static"
  | CIexpr _ -> "" (* <expr>::class is banned *)
  | CI (_, name) -> name
  | CIreified (_, name) -> name

let check_string_coercion_point env expr ty =
  if env.Typing_env_types.emit_string_coercion_error then
    match expr with
    | (_, pos, Class_const ((_, _, cid_), (_, cls)))
      when cls = Naming_special_names.Members.mClass -> begin
      match cid_ with
      | CIreified _ -> () (* nameof T is illegal (T187575261) *)
      | _ ->
        let check ty =
          match get_node ty with
          | Tclass ((_, id), _, _)
            when String.equal id Naming_special_names.Classes.cString ->
            error_at_cls_const_expr env pos (string_of_class_id_ cid_)
          | Tprim Tarraykey ->
            error_at_cls_const_expr env pos (string_of_class_id_ cid_)
          | _ -> ()
        in
        let (_env, ty) = Typing_dynamic_utils.strip_dynamic env ty in
        begin
          match get_node ty with
          | Toption ty -> check ty
          | _ -> check ty
        end
    end
    | _ -> ()

(* Weaken class-typed expression at runtime string coercion points *)
let rec coerce_to_name env ty =
  let (env, ety) = Typing_env.expand_type env ty in
  let (stripped, env, inner) = Typing_utils.strip_supportdyn env ety in
  let rewrap ty =
    if stripped then
      Typing_make_type.supportdyn (get_reason ety) ty
    else
      ty
  in
  match deref inner with
  | (r, Tclass_ptr ty_inner) ->
    (env, rewrap (Typing_make_type.classname r [ty_inner]))
  | (r, Tunion tyl) ->
    let (env, tyl) = Common.List.map_env env tyl ~f:coerce_to_name in
    (env, rewrap (mk (r, Tunion tyl)))
  | (r, Tintersection tyl) ->
    let (env, tyl) = Common.List.map_env env tyl ~f:coerce_to_name in
    (env, rewrap (mk (r, Tintersection tyl)))
  | (r, (Tgeneric _ | Tnewtype _)) ->
    let (env, bound) =
      Typing_utils.get_base_type ~expand_supportdyn:false env inner
    in
    begin
      match deref bound with
      | (_, Tclass_ptr ty_inner) ->
        (env, rewrap (Typing_make_type.classname r [ty_inner]))
      | _ -> (env, ty)
    end
  | _ -> (env, ty)

(* Whether a value of [ty] can be, or contain as a container value, a string
   that isn't a class pointer. Like types are trusted, as in refinement. *)
let rec may_hold_string ?(expanding = []) env ty =
  let (env, ety) = Typing_env.expand_type env ty in
  let (_, env, ety) = Typing_utils.strip_supportdyn env ety in
  let exists ~expanding env tyl =
    Common.List.fold_left tyl ~init:(env, false) ~f:(fun (env, found) ty ->
        if found then
          (env, true)
        else
          may_hold_string ~expanding env ty)
  in
  let for_all ~expanding env tyl =
    Common.List.fold_left tyl ~init:(env, true) ~f:(fun (env, all) ty ->
        if not all then
          (env, false)
        else
          may_hold_string ~expanding env ty)
  in
  match get_node ety with
  | Tclass_ptr _
  | Tfun _
  | Tlabel _
  | Tdynamic _ ->
    (env, false)
  | Tprim Tarraykey -> (env, true)
  | Tprim _ -> (env, false)
  | Tclass ((_, name), _, tyl) ->
    (* A string is also an instance of some interfaces, e.g. Stringish *)
    if
      String.equal name Naming_special_names.Classes.cString
      || Typing_utils.is_sub_type_for_union
           env
           (Typing_make_type.string Typing_reason.none)
           ety
    then
      (env, true)
    else (
      (* The value type of vec<T>, dict<K, T>, KeyedContainer<K, T>, ... *)
      match Common.List.last tyl with
      | Some ty -> may_hold_string ~expanding env ty
      | None -> (env, false)
    )
  | Toption ty -> may_hold_string ~expanding env ty
  | Tunion tyl -> exists ~expanding env tyl
  | Tintersection tyl -> for_all ~expanding env tyl
  | Tvec_or_dict (_, ty) -> may_hold_string ~expanding env ty
  | Ttuple { t_required; t_optional; t_extra = Tvariadic ty | Tsplat ty } ->
    exists ~expanding env ((ty :: t_required) @ t_optional)
  | Tshape (Shape_simple { s_unknown_value; s_fields; _ }) ->
    let field_tys =
      TShapeMap.fold (fun _ sft acc -> sft.sft_ty :: acc) s_fields []
    in
    exists ~expanding env (s_unknown_value :: field_tys)
  | Tshape (Shape_splat { ss_elems }) -> exists ~expanding env ss_elems
  | Tgeneric _
  | Tnewtype _
  | Tdependent _
    when not (Common.List.mem expanding ety ~equal:ty_equal) ->
    let (env, bounds) =
      Typing_utils.get_concrete_supertypes
        ~expand_supportdyn:false
        ~abstract_enum:false
        env
        ety
    in
    (match bounds with
    | [] -> (env, true)
    | _ -> for_all ~expanding:(ety :: expanding) env bounds)
  | Tnonnull
  | Tany _
  | Tvar _
  | Tneg _
  | Taccess _
  | Tgeneric _
  | Tnewtype _
  | Tdependent _ ->
    (env, true)
