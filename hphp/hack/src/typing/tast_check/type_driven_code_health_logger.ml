(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SN = Naming_special_names
module T = Typing_defs

type kind =
  | HH_FIXME
  | UNSAFE_CAST
  | PHPISM
  | EXPLICIT_DYNAMIC
  | TYPE_ASSERTION
  | UNSURFACED_EXCEPTION
  | ALWAYS_FALSE_ASSERTION
  | ALWAYS_FALSE_TYPE_TEST
[@@deriving show { with_path = false }]

let find_level = function
  | HH_FIXME
  | UNSAFE_CAST
  | PHPISM
  | EXPLICIT_DYNAMIC
  | TYPE_ASSERTION ->
    1
  | UNSURFACED_EXCEPTION
  | ALWAYS_FALSE_ASSERTION
  | ALWAYS_FALSE_TYPE_TEST ->
    2

let nPHPism_FIXME = "\\PHPism_FIXME"

let log level pos kind =
  if find_level kind <= level then
    let json =
      Hh_json.JSON_Object
        [
          ("kind", Hh_json.JSON_String (show_kind kind));
          ("pos", Pos.multiline_json (Pos.to_relative_string pos));
        ]
    in
    Hh_logger.log "[TDCH] %s" (Hh_json.json_to_string json)

let nothing_ty = Typing_make_type.nothing Typing_reason.none

let replace_placeholders_with_tvars env ty =
  let replace_placeholder env ty =
    match T.get_node ty with
    | T.Tgeneric name when String.contains name '#' ->
      Typing_env.fresh_type env Pos.none
    | _ -> (env, ty)
  in
  match T.get_node ty with
  | T.Tclass (id, exact, targs) ->
    let (env, targs) = List.fold_map ~f:replace_placeholder ~init:env targs in
    (env, T.mk (Typing_reason.none, T.Tclass (id, exact, targs)))
  | _ -> (env, ty)

let always_false level pos kind env lhs_ty rhs_ty =
  let (env, lhs_ty) = Tast_env.expand_type env lhs_ty in
  let (env, rhs_ty) = Tast_env.expand_type env rhs_ty in
  let tenv = Tast_env.tast_env_as_typing_env env in
  let lhs_ty = Tast_env.strip_dynamic env lhs_ty in
  let rhs_ty = Tast_env.strip_dynamic env rhs_ty in
  let tenv = Typing_env.open_tyvars tenv Pos.none in
  let (tenv, rhs_ty) = replace_placeholders_with_tvars tenv rhs_ty in
  if Tast_env.is_sub_type env lhs_ty nothing_ty then
    (* If we have a nothing in our hands, there was a bigger problem
       originating from earlier in the program. Don't flag it here, as it is
       merely a symptom. *)
    ()
  else
    let env = Tast_env.typing_env_as_tast_env tenv in
    let (env, lhs_ty) = Tast_env.expand_type env lhs_ty in
    let (_env, rhs_ty) = Tast_env.expand_type env rhs_ty in
    if Typing_utils.is_type_disjoint tenv lhs_ty rhs_ty then log level pos kind

let is_toplevelish_dynamic env (ty, hint_opt) =
  match hint_opt with
  | Some (pos, _) ->
    let rec is_toplevelish_dynamic ty =
      let (_env, ty) = Tast_env.expand_type env ty in
      let ty = Tast_env.strip_dynamic env ty in
      let (_env, ty) = Tast_env.expand_type env ty in
      match T.get_node ty with
      | T.Tdynamic -> true
      | T.Toption ty -> is_toplevelish_dynamic ty
      | T.Tclass ((_, id), _, [ty]) when String.equal id SN.Classes.cAwaitable
        ->
        is_toplevelish_dynamic ty
      | T.Tclass ((_, id), _, [ty])
        when List.mem
               ~equal:String.equal
               [SN.Collections.cVec; SN.Collections.cVector]
               id ->
        is_toplevelish_dynamic ty
      | T.Tclass ((_, id), _, [_; ty])
        when List.mem
               ~equal:String.equal
               [SN.Collections.cDict; SN.Collections.cMap]
               id ->
        is_toplevelish_dynamic ty
      | _ -> false
    in
    if is_toplevelish_dynamic ty then
      Some pos
    else
      None
  | _ -> None

let log_explicit_dynamic level env hint =
  match is_toplevelish_dynamic env hint with
  | Some pos -> log level pos EXPLICIT_DYNAMIC
  | None -> ()

let log_callable_def level env ret params =
  log_explicit_dynamic level env ret;
  List.iter params ~f:(fun param ->
      log_explicit_dynamic level env param.Aast.param_type_hint)

let generic_exceptions =
  [
    "\\Exception";
    "\\ViolationException";
    "\\InvariantViolationException";
    "\\InvalidOperationException";
    "\\RuntimeException";
  ]

let references bl binding =
  let references =
    object
      inherit [_] Aast.reduce

      method zero = false

      method plus = ( || )

      method! on_lid _ (_, lid) = Local_id.equal binding lid
    end
  in
  references#on_block () bl

let log_on_unsurfaced_exception level ((_, exn), (pos, exn_binding), bl) =
  let is_placeholder lid =
    String.equal (Local_id.get_name lid) SN.SpecialIdents.placeholder
  in
  if
    List.exists
      ~f:(fun generic_exn -> String.is_suffix ~suffix:generic_exn exn)
      generic_exceptions
    && (is_placeholder exn_binding || not (references bl exn_binding))
  then
    log level pos UNSURFACED_EXCEPTION

let create_handler ctx =
  let level =
    Provider_context.get_tcopt ctx
    |> TypecheckerOptions.log_levels
    |> SMap.find_opt "type_driven_code_health"
    |> Option.value ~default:1
  in
  object
    inherit Tast_visitor.handler_base

    method! at_fun_ env Aast.{ f_ret; f_params; _ } =
      log_callable_def level env f_ret f_params

    method! at_method_ env Aast.{ m_ret; m_params; _ } =
      log_callable_def level env m_ret m_params

    method! at_expr env (_, pos, expr) =
      let log = log level pos in
      match expr with
      | Aast.Hole (_, _, _, kind) -> begin
        match kind with
        | Aast.Typing -> log HH_FIXME
        | Aast.UnsafeCast _ -> log UNSAFE_CAST
        | Aast.UnsafeNonnullCast -> log UNSAFE_CAST
        | _ -> ()
      end
      | Aast.As
          Aast.
            {
              expr = (lhs_ty, _, _);
              hint;
              is_nullable = false;
              enforce_deep = _;
            } ->
        log TYPE_ASSERTION;
        let (env, hint_ty) = Tast_env.localize_hint_for_refinement env hint in
        always_false level pos ALWAYS_FALSE_ASSERTION env lhs_ty hint_ty
      | Aast.Is ((lhs_ty, _, _), hint) ->
        let (env, hint_ty) = Tast_env.localize_hint_for_refinement env hint in
        always_false level pos ALWAYS_FALSE_TYPE_TEST env lhs_ty hint_ty
      | Aast.Call
          Aast.
            { func = (_, _, Aast.Class_const ((_, _, Aast.CI (_, cid)), _)); _ }
        when String.equal cid nPHPism_FIXME ->
        log PHPISM
      | _ -> ()

    method! at_stmt _env (_, stmt) =
      match stmt with
      | Aast.Try (_, catch_list, _) ->
        List.iter ~f:(log_on_unsurfaced_exception level) catch_list
      | _ -> ()
  end
