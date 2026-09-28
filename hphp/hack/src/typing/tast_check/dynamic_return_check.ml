(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs

let warning_kind = Typing_warning.Dynamic_return

let error_codes = Typing_warning_utils.codes warning_kind

let return_value_type fun_kind ty =
  match (fun_kind, get_node ty) with
  | (Ast_defs.FAsync, Tapply ((_, name), [inner_ty]))
    when String.equal name Naming_special_names.Classes.cAwaitable ->
    inner_ty
  | (Ast_defs.FAsync, Tlike ty) -> begin
    match get_node ty with
    | Tapply ((_, name), [inner_ty])
      when String.equal name Naming_special_names.Classes.cAwaitable ->
      mk (get_reason ty, Tlike inner_ty)
    | _ -> ty
  end
  | _ -> ty

let check_return env return_type (expr_ty, expr_pos, _) =
  let (_env, expr_ty) = Tast_env.expand_type env expr_ty in
  match get_node expr_ty with
  | Tdynamic _ ->
    let return_type_pos = get_pos return_type in
    let return_type = Tast_env.print_decl_ty env return_type in
    Tast_env.add_warning
      env
      ( expr_pos,
        Typing_warning.Dynamic_return,
        { Typing_warning.Dynamic_return.return_type; return_type_pos } )
  | _ -> ()

let check_callable env fun_kind type_hint body =
  match hint_of_type_hint type_hint with
  | None -> ()
  | Some hint ->
    let return_type =
      Tast_env.hint_to_ty env hint |> return_value_type fun_kind
    in
    if not (is_dynamic return_type) then
      let this_class = Tast_env.get_self_class env |> Decl_entry.to_option in
      let Equal = Tast_env.eq_typing_env in
      let is_enforced =
        match
          Typing_enforceability.get_enforcement
            ~top_enforced:true
            ~this_class
            env
            return_type
        with
        | Enforced -> true
        | Unenforced ->
          let enforced_type =
            Typing_partial_enforcement.get_enforced_type
              env
              this_class
              return_type
          in
          Typing_phase.is_sub_type_decl
            ~is_dynamic_aware:true
            env
            enforced_type
            return_type
      in
      if not is_enforced then
        let visitor =
          object
            inherit [_] Aast.iter as super

            method! on_stmt env stmt =
              (match stmt with
              | (_, Return (Some expr)) -> check_return env return_type expr
              | _ -> ());
              super#on_stmt env stmt

            method! on_fun_ _ _ = ()

            method! on_method_ _ _ = ()
          end
        in
        visitor#on_block env body.fb_ast

let handler ~as_lint:_ =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_ env fun_ =
      let env = Tast_env.restore_fun_env env fun_ in
      check_callable env fun_.f_fun_kind fun_.f_ret fun_.f_body

    method! at_method_ env method_ =
      let env = Tast_env.restore_method_env env method_ in
      check_callable env method_.m_fun_kind method_.m_ret method_.m_body
  end
