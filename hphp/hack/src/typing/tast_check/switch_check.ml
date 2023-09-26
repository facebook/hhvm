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
open Utils
module Env = Tast_env
module Cls = Decl_provider.Class
module SN = Naming_special_names
module MakeType = Typing_make_type
module Reason = Typing_reason

(* This is used for a heuristic to handle a common case in enum refinements.
   So, it identifies the common enum bounds, namely, arraykey, string, and int.

   I am deliberately not using subtyping because it causes the heuristic to
   capture intersections of two enums. *)
let is_common_enum_bound env ty =
  let (_env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tprim Ast_defs.(Tarraykey | Tint | Tstring) -> true
  | _ -> false

let is_dynamic env ty = Env.is_sub_type env ty (MakeType.dynamic Reason.Rnone)

let is_enum env ty =
  let (env, ty) = Env.expand_type env ty in
  match Typing_defs.get_node ty with
  | Tnewtype (name, _, _) -> Env.is_enum env name
  | _ -> false

let is_like_enum env ty =
  let (env, ty) = Env.expand_type env ty in
  match Typing_defs.get_node ty with
  | Typing_defs.Tunion [ty; dynamic_ty] when is_dynamic env dynamic_ty ->
    is_enum env ty
  | Typing_defs.Tunion [dynamic_ty; ty] when is_dynamic env dynamic_ty ->
    is_enum env ty
  | _ -> false

let get_constant env tc kind (seen, has_default) case =
  let (kind, is_enum_class_label) =
    match kind with
    | If_enum_or_enum_class.Enum -> ("enum ", false)
    | If_enum_or_enum_class.EnumClass -> ("enum class ", false)
    | If_enum_or_enum_class.EnumClassLabel -> ("enum class ", true)
  in
  let check_case pos cls const =
    (* wish:T109260699 *)
    if String.( <> ) cls (Cls.name tc) then (
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          enum
          @@ Primary.Enum.Enum_switch_wrong_class
               {
                 pos;
                 kind;
                 expected = lazy (strip_ns (Cls.name tc));
                 actual = lazy (strip_ns cls);
                 expected_pos = None;
               });
      (seen, has_default)
    ) else
      match SMap.find_opt const seen with
      | None -> (SMap.add const pos seen, has_default)
      | Some old_pos ->
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            enum
            @@ Primary.Enum.(
                 Enum_switch_redundant
                   {
                     const_name = Const.(Label { class_ = cls; const });
                     first_pos = old_pos;
                     pos;
                   }));

        (seen, has_default)
  in
  match case with
  | Default _ -> (seen, true)
  | Case ((_, pos, Class_const ((_, _, CI (_, cls)), (_, const))), _)
    when not is_enum_class_label ->
    check_case pos cls const
  | Case ((_, pos, EnumClassLabel (Some (_, cls), const)), _) ->
    check_case pos cls const
  | Case ((_, pos, _), _) ->
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(enum @@ Primary.Enum.Enum_switch_not_const pos);
    (seen, has_default)

let check_enum_exhaustiveness
    env pos tc kind (caselist, dfl) coming_from_unresolved =
  let str_kind =
    match kind with
    | If_enum_or_enum_class.Enum -> "Enum"
    | If_enum_or_enum_class.EnumClass
    | If_enum_or_enum_class.EnumClassLabel ->
      "Enum class"
  in
  (* If this check comes from an enum inside a Tunion, then
     don't punish for having an extra default case *)
  let (seen, has_default) =
    let state = (SMap.empty, false) in
    let state =
      List.fold_left
        ~f:(fun state c -> get_constant env tc kind state (Aast.Case c))
        ~init:state
        caselist
    in
    let state =
      Option.fold
        ~f:(fun state c -> get_constant env tc kind state (Aast.Default c))
        ~init:state
        dfl
    in
    state
  in
  let rec first_n_unhandled n acc = function
    | _ when n < 1 -> List.rev acc
    | [] -> List.rev acc
    | (id, _) :: rest ->
      if String.equal id SN.Members.mClass || SMap.mem id seen then
        first_n_unhandled n acc rest
      else
        first_n_unhandled (n - 1) (id :: acc) rest
  in
  let unhandled = first_n_unhandled 10 [] @@ Cls.consts tc in
  let class_ = Cls.name tc in
  let all_cases_handled = List.is_empty unhandled in
  let enum_err_opt =
    let open Typing_error in
    match (all_cases_handled, has_default, coming_from_unresolved) with
    | (false, false, _) ->
      Some
        Primary.Enum.(
          Enum_switch_nonexhaustive
            {
              pos;
              kind = Some str_kind;
              missing =
                List.map
                  ~f:(fun x -> Const.(Some (Label { class_; const = x })))
                  unhandled;
              decl_pos = Cls.pos tc;
            })
    | (true, true, false) ->
      Some
        (Primary.Enum.Enum_switch_redundant_default
           { pos; kind = str_kind; decl_pos = Cls.pos tc })
    | _ -> None
  in
  Option.iter enum_err_opt ~f:(fun err ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
      @@ Typing_error.enum err)

(* Small reminder:
 * - enums are localized to `Tnewtype (name, _, _)` where name is the name of
 *   the enum. This can be checked using `Env.is_enum`
 * - enum classes are not inhabited by design. The type of elements is
 *   HH\MemberOf<name, interface> were name is the name of the enum class. This
 *   is localized as Tnewtype("HH\MemberOf", [enum; interface]) where
 *   enum is localized as Tclass(name, _, _) where Env.is_enum_class name is
 *   true.
 *)
(* Wrapper to share the logic that detects if a type is an enum or an enum
 * class, or something else.
 *)
let rec check_exhaustiveness_
    env pos ty caselist enum_coming_from_unresolved ~outcomes =
  (* Right now we only do exhaustiveness checking for enums. *)
  (* This function has a built in hack where if Tunion has an enum
     inside then it tells the enum exhaustiveness checker to
     not punish for extra default *)
  let (env, ty) = Env.expand_type env ty in
  let check kind env id ~outcomes =
    let dep = Typing_deps.Dep.AllMembers id in
    let decl_env = Env.get_decl_env env in
    Option.iter decl_env.Decl_env.droot ~f:(fun root ->
        Typing_deps.add_idep (Env.get_deps_mode env) root dep);
    if TypecheckerOptions.record_fine_grained_dependencies @@ Env.get_tcopt env
    then
      Typing_pessimisation_deps.try_add_fine_dep
        (Env.get_deps_mode env)
        decl_env.Decl_env.droot
        decl_env.Decl_env.droot_member
        dep;

    let tc = unsafe_opt @@ Env.get_enum env id in
    check_enum_exhaustiveness
      env
      pos
      tc
      kind
      caselist
      enum_coming_from_unresolved;
    (`Enum_checked :: outcomes, env)
  in
  match get_node ty with
  | Tunion tyl ->
    let new_enum =
      enum_coming_from_unresolved
      || List.length tyl > 1
         && List.exists tyl ~f:(fun cur_ty ->
                let (_, cur_ty) = Env.expand_type env cur_ty in
                match get_node cur_ty with
                | Tnewtype (name, args, _) ->
                  If_enum_or_enum_class.apply
                    env
                    ~default:false
                    ~f:(fun _ _ _ -> true)
                    name
                    args
                | _ -> false)
    in
    List.fold_left tyl ~init:(outcomes, env) ~f:(fun (outcomes, env) ty ->
        check_exhaustiveness_ env pos ty caselist new_enum ~outcomes)
  | Tintersection [enum_bound; like_ty]
    when is_common_enum_bound env enum_bound && is_like_enum env like_ty ->
    check_exhaustiveness_
      env
      pos
      like_ty
      caselist
      enum_coming_from_unresolved
      ~outcomes
  | Tintersection [like_ty; enum_bound]
    when is_common_enum_bound env enum_bound && is_like_enum env like_ty ->
    check_exhaustiveness_
      env
      pos
      like_ty
      caselist
      enum_coming_from_unresolved
      ~outcomes
  | Tintersection tyl ->
    fst
    @@ Typing_utils.run_on_intersection
         (outcomes, env)
         tyl
         ~f:(fun (outcomes, env) ty ->
           ( check_exhaustiveness_
               env
               pos
               ty
               caselist
               enum_coming_from_unresolved
               ~outcomes,
             () ))
  | Tnewtype (name, args, _) ->
    If_enum_or_enum_class.apply
      env
      ~default:(outcomes, env)
      ~f:(check ~outcomes)
      name
      args
  | Tany _
  | Tnonnull
  | Tvec_or_dict _
  | Tclass _
  | Toption _
  | Tprim _
  | Tvar _
  | Tfun _
  | Tgeneric _
  | Tdependent _
  | Ttuple _
  | Tshape _
  | Taccess _
  | Tneg _
  | Tdynamic ->
    if Option.is_none (snd caselist) then
      (`Silently_ends ty :: outcomes, env)
    else
      (`Dyn_with_default :: outcomes, env)
  | Tunapplied_alias _ ->
    Typing_defs.error_Tunapplied_alias_in_illegal_context ()

type outcomes = {
  silently_ends: Tast.ty list;
  dyn_with_default: int;
  enum_checked: int;
}

let count_outcomes =
  let f
      ({ silently_ends; dyn_with_default; enum_checked } as outcomes : outcomes)
      = function
    | `Silently_ends ty -> { outcomes with silently_ends = ty :: silently_ends }
    | `Dyn_with_default ->
      { outcomes with dyn_with_default = dyn_with_default + 1 }
    | `Enum_checked -> { outcomes with enum_checked = enum_checked + 1 }
  in
  List.fold_left
    ~f
    ~init:{ silently_ends = []; dyn_with_default = 0; enum_checked = 0 }

let rec get_kind_and_args json =
  let open Option.Let_syntax in
  let* kind = Hh_json.get_field_opt (Hh_json.Access.get_string "kind") json in
  match kind with
  | "primitive" ->
    let* name = Hh_json.get_field_opt (Hh_json.Access.get_string "name") json in
    Some ("prim_" ^ name)
  | "nullable" ->
    let* args = Hh_json.get_field_opt (Hh_json.Access.get_array "args") json in
    let* hd = List.hd args in
    let* recurse = get_kind_and_args hd in
    Some ("nullable_" ^ recurse)
  | _ -> Some kind

let to_json_array env silently_ends =
  silently_ends
  |> List.filter_map ~f:(fun ty ->
         ty |> Env.ty_to_json env ~show_like_ty:true |> get_kind_and_args)
  |> List.dedup_and_sort ~compare:String.compare
  |> Hh_json.array_ Hh_json.string_

let outcomes_to_fields
    env ({ silently_ends; dyn_with_default; enum_checked } : outcomes) =
  let silently_ends = ("silently_ends", to_json_array env silently_ends) in
  let dyn_with_default = ("dyn_with_default", Hh_json.int_ dyn_with_default) in
  let enum_checked = ("enum_checked", Hh_json.int_ enum_checked) in
  [silently_ends; dyn_with_default; enum_checked]

let log_exhaustivity_check env pos default_label outcomes ty_json =
  let add_fields json ~fields =
    match json with
    | Hh_json.JSON_Object old -> Hh_json.JSON_Object (old @ fields)
    | _ -> Hh_json.JSON_Object (("warning_expected_object", json) :: fields)
  in
  let has_default =
    ("has_default", Option.is_some default_label |> Hh_json.bool_)
  in
  let switch_pos = ("switch_pos", Pos.(pos |> to_absolute |> json)) in
  let fields =
    has_default
    :: switch_pos
    :: (outcomes_to_fields env @@ count_outcomes outcomes)
  in
  ty_json
  |> add_fields ~fields
  |> Hh_json.json_to_string
  |> Hh_logger.log "[hh_tco_log_exhaustivity_check] %s"

let check_exhaustiveness env pos ty ((_, dfl) as caselist) =
  let (outcomes, env) =
    check_exhaustiveness_ env pos ty caselist false ~outcomes:[]
  in
  let tcopt = env |> Env.get_decl_env |> Decl_env.tcopt in
  if TypecheckerOptions.tco_log_exhaustivity_check tcopt then
    Env.ty_to_json env ~show_like_ty:true ty
    |> log_exhaustivity_check env pos dfl outcomes

let handler =
  object
    inherit Tast_visitor.handler_base

    val mutable strict_switch_function_or_method = false

    method! at_fun_ _env (f : Tast.fun_) =
      strict_switch_function_or_method <-
        Naming_attributes.mem
          Naming_special_names.UserAttributes.uaStrictSwitch
          f.f_user_attributes

    method! at_method_ _env (m : Tast.method_) =
      strict_switch_function_or_method <-
        Naming_attributes.mem
          Naming_special_names.UserAttributes.uaStrictSwitch
          m.m_user_attributes

    method! at_stmt env x =
      match snd x with
      | Switch ((scrutinee_ty, scrutinee_pos, _), casel, dfl) ->
        if strict_switch_function_or_method then (
          Strict_switch_int_literal_check.handler#at_stmt env x;
          Strict_switch_check.handler#at_stmt env x
        ) else
          check_exhaustiveness env scrutinee_pos scrutinee_ty (casel, dfl)
      | _ -> ()
  end
