(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Bad_type_logger_types
open Bad_type_logger_file
open Bad_type_logger_scuba

(* A bad type is a Tany or Terr *)

let log_info env (infos : info list) : unit =
  List.iter ~f:(log_info_to_file env) infos;
  log_info_to_scuba env infos

let extract_bad_type_indicator ty =
  let finder =
    object
      inherit [bad_type_indicator] Type_visitor.locl_type_visitor

      method! on_tany indicator _ = { indicator with has_tany = true }

      method! on_terr indicator _ = { indicator with has_terr = true }
    end
  in
  let init_indicator = { has_tany = false; has_terr = false } in
  finder#on_type init_indicator ty

let mk_common_info ~context_id ~indicator pos =
  let path = Pos.filename pos |> Relative_path.suffix in
  let is_generated = String_utils.is_substring "generated" path in
  let is_test = String_utils.is_substring "test" path in
  { context_id; indicator; is_generated; is_test; pos }

let mk_ret_decl_info_internal ~context_id ~indicator env ty =
  let pos =
    Typing_defs.get_pos ty
    |> Pos_or_decl.fill_in_filename (Tast_env.get_file env)
  in
  let common_info = mk_common_info ~context_id ~indicator pos in
  let position = Return in
  let context = Declaration { position } in
  { common_info; context }

let mk_ret_decl_info
    ~context_id env ((ty, _hint) : Typing_defs.locl_ty Aast.type_hint) =
  let indicator = extract_bad_type_indicator ty in
  if has_bad_type indicator then
    let ret_decl_info =
      mk_ret_decl_info_internal ~context_id ~indicator env ty
    in
    Some ret_decl_info
  else
    None

let mk_param_decl_info_internal ~context_id ~indicator ~is_variadic pos callconv
    =
  let common_info = mk_common_info ~context_id ~indicator pos in
  let is_inout =
    match callconv with
    | Some Ast_defs.Pinout -> true
    | _ -> false
  in
  let position = Parameter { is_inout; is_variadic } in
  let context = Declaration { position } in
  { common_info; context }

let mk_param_decl_info
    ~context_id
    ~is_variadic
    Aast.{ param_pos; param_callconv; param_type_hint = (ty, _); _ } =
  let indicator = extract_bad_type_indicator ty in
  if has_bad_type indicator then
    let param_decl_info =
      mk_param_decl_info_internal
        ~context_id
        ~indicator
        ~is_variadic
        param_pos
        param_callconv
    in
    Some param_decl_info
  else
    None

let mk_callable_decl_infos ~context_id env variadicity params ret =
  let variadic_decl_param_info =
    let is_variadic = true in
    match variadicity with
    | Aast.FVvariadicArg param ->
      mk_param_decl_info ~context_id ~is_variadic param
    | _ -> None
  in
  let non_variadic_param_infos =
    List.map ~f:(mk_param_decl_info ~context_id ~is_variadic:false) params
  in
  List.filter_opt
  @@ mk_ret_decl_info ~context_id env ret
     :: variadic_decl_param_info :: non_variadic_param_infos

let mk_expr_info_internal ~context_id ~indicator pos ty exp =
  let common_info = mk_common_info ~context_id ~indicator pos in
  let producer_id = function
    | Aast.Id (_, id) -> Some id
    | Aast.Class_const ((_, _, Aast.CI (_, class_id)), (_, id)) ->
      Some (class_id ^ "::" ^ id)
    | _ -> None
  in
  let exp_info =
    match exp with
    | Aast.Call ((_, _, receiver), _, _, _) ->
      let producer =
        match producer_id receiver with
        | Some id -> Some id
        | None -> Some "-"
      in
      let declaration_usage = false in
      { producer; declaration_usage }
    | Aast.Id _
    | Aast.Class_const _ ->
      let producer = None in
      let declaration_usage =
        match Typing_defs.get_node ty with
        | Typing_defs.Tfun _ -> true
        | _ -> false
      in
      { producer; declaration_usage }
    | _ ->
      let producer = None in
      let declaration_usage = false in
      { producer; declaration_usage }
  in
  let context = Expression exp_info in
  { common_info; context }

let mk_expr_info ~context_id (ty, pos, exp) =
  let indicator = extract_bad_type_indicator ty in
  if has_bad_type indicator then
    let expr_info = mk_expr_info_internal ~context_id ~indicator pos ty exp in
    Some expr_info
  else
    None

(** Reduction to collect all Tany/Terr information below methods and functions.*)
let collect_infos_methods_functions context_id =
  object
    inherit [_] Tast_visitor.reduce as super

    method zero = []

    method plus = ( @ )

    method! on_method_ env (Aast.{ m_params; m_ret; m_variadic; _ } as m) =
      let infos =
        mk_callable_decl_infos ~context_id env m_variadic m_params m_ret
      in
      infos @ super#on_method_ env m

    method! on_fun_ env (Aast.{ f_params; f_ret; f_variadic; _ } as f) =
      let infos =
        mk_callable_decl_infos ~context_id env f_variadic f_params f_ret
      in
      infos @ super#on_fun_ env f

    method! on_expr env exp =
      let infos =
        match mk_expr_info ~context_id exp with
        | Some info -> [info]
        | None -> []
      in
      infos @ super#on_expr env exp
  end

(** Reduction to collect all Tany/Terr information. It calls the reudction for
    functions and methods with the context information attached. *)
let collect_infos =
  object
    inherit [_] Tast_visitor.reduce

    method zero = []

    method plus = ( @ )

    method! on_fun_ env (Aast.{ f_name; _ } as f) =
      let context_id = Function (snd f_name) in
      (collect_infos_methods_functions context_id)#on_fun_ env f

    method! on_method_ env (Aast.{ m_name; _ } as m) =
      let context_id =
        match Tast_env.get_self_id env with
        | Some c_name -> Method (c_name, snd m_name)
        | None -> failwith "method does not have an enclosing class"
      in
      (collect_infos_methods_functions context_id)#on_method_ env m
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_ env f =
      let infos = collect_infos#on_fun_ env f in
      log_info env infos

    method! at_class_ env c =
      let infos = collect_infos#on_class_ env c in
      log_info env infos
  end
