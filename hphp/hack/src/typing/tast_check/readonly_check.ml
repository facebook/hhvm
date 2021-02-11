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
module Env = Tast_env
module Cls = Decl_provider.Class
module SN = Naming_special_names

type rty =
  | Readonly
  | Mut [@deriving show]

type ctx = {
  lenv: rty SMap.t;
  (* whether the method/function returns readonly, and a Pos.t for error messages *)
  ret_ty: (rty * Pos.t) option;
  (* Whether $this is readonly and a Pos.t for error messages *)
  this_ty: (rty * Pos.t) option;
}

let empty_ctx = { lenv = SMap.empty; ret_ty = None; this_ty = None }

let readonly_kind_to_rty = function
  | Some Ast_defs.Readonly -> Readonly
  | _ -> Mut

let rty_to_str = function
  | Readonly -> "readonly"
  | Mut -> "mutable"

let lenv_from_params (params : Tast.fun_param list) : rty SMap.t =
  let result = SMap.empty in
  List.fold_left
    params
    ~f:(fun acc p ->
      SMap.add p.param_name (readonly_kind_to_rty p.param_readonly) acc)
    ~init:result

let get_local lenv id =
  match SMap.find_opt id lenv with
  | Some r -> r
  | None -> Mut

(* Returns true if rty_sub is a subtype of rty_sup.
TODO: Later, we'll have to consider the regular type as well, for example
we could allow readonly int as equivalent to an int for devX purposes *)
let subtype_rty rty_sub rty_sup =
  match (rty_sub, rty_sup) with
  | (Readonly, Mut) -> false
  | _ -> true

let param_to_rty param =
  if Typing_defs.get_fp_readonly param then
    Readonly
  else
    Mut

let check =
  object (self)
    inherit Tast_visitor.iter as super

    val mutable ctx : ctx = empty_ctx

    method ty_expr (e : Tast.expr) : rty =
      match e with
      | (_, ReadonlyExpr _) -> Readonly
      | (_, This) ->
        (match ctx.this_ty with
        | Some (r, _) -> r
        | None -> Mut)
      | (_, Lvar (_, lid)) ->
        let varname = Local_id.to_string lid in
        get_local ctx.lenv varname
      (* If you have a bunch of property accesses in a row, i.e. $x->foo->bar->baz,
        ty_expr will take linear time, and the full check may take O(n^2) time
        if we recurse on expressions in the visitor. We expect this to generally
        be quite small, though. *)
      | (_, Obj_get (e1, _, _, _)) -> self#ty_expr e1
      | _ -> Mut

    method assign lval rval =
      match lval with
      | (_, Obj_get (e1, _, _, _)) ->
        begin
          match self#ty_expr e1 with
          | Readonly -> Errors.readonly_modified (Tast.get_position e1)
          | Mut -> ()
        end
      | (_, Lvar (_, lid)) ->
        let var_ro_opt = SMap.find_opt (Local_id.to_string lid) ctx.lenv in
        begin
          match (var_ro_opt, self#ty_expr rval) with
          | (Some Readonly, Mut) ->
            Errors.var_readonly_mismatch
              (Tast.get_position lval)
              "readonly"
              (Tast.get_position rval)
              "mutable"
          | (Some Mut, Readonly) ->
            Errors.var_readonly_mismatch
              (Tast.get_position lval)
              "mutable"
              (Tast.get_position rval)
              "readonly"
          | (None, r) ->
            (* If it's a new assignment, add to the lenv *)
            let new_lenv = SMap.add (Local_id.to_string lid) r ctx.lenv in
            ctx <- { ctx with lenv = new_lenv }
          | (Some Mut, Mut) -> ()
          | (Some Readonly, Readonly) -> ()
        end
      (* TODO: awaitables *)
      (* TODO: vecs, collections, array accesses *)
      | _ -> ()

    (* Checks related to calling a function or method
      is_readonly is true when the call is allowed to return readonly
      TODO: handle inout
    *)
    method call
        ~is_readonly
        (caller : Tast.expr)
        (args : Tast.expr list)
        (unpacked_arg : Tast.expr option) =
      let open Typing_defs in
      (* Check that function calls which return readonly are wrapped in readonly *)
      let check_readonly_call caller is_readonly =
        match get_node (Tast.get_type caller) with
        | Tfun fty when get_ft_returns_readonly fty ->
          if not is_readonly then
            Errors.explicit_readonly_cast
              "function call"
              (Tast.get_position caller)
        | _ -> ()
      in
      (* Check that readonly arguments match their parameters *)
      let check_args caller args unpacked_arg =
        match get_node (Tast.get_type caller) with
        | Tfun fty ->
          let unpacked_rty = Option.to_list unpacked_arg in
          let args = args @ unpacked_rty in
          (* If the args are unequal length, we errored elsewhere so this does not care *)
          let _ =
            List.iter2 fty.ft_params args ~f:(fun param arg ->
                let param_rty = param_to_rty param in
                let arg_rty = self#ty_expr arg in
                if not (subtype_rty arg_rty param_rty) then
                  Errors.readonly_mismatch
                    "Invalid argument"
                    (Tast.get_position arg)
                    ~reason_sub:
                      [
                        ( Tast.get_position arg,
                          "This expression is " ^ rty_to_str arg_rty );
                      ]
                    ~reason_super:
                      [
                        ( param.fp_pos,
                          "It is incompatible with this parameter, which is "
                          ^ rty_to_str param_rty );
                      ])
          in
          ()
        | _ -> ()
      in
      (* Check that a RO expression can only call RO methods *)
      let check_method_call caller =
        match caller with
        (* Method call checks *)
        | ((_, ty), Obj_get (e1, _, _, (* is_prop_call *) false)) ->
          let receiver_rty = self#ty_expr e1 in
          (match (receiver_rty, get_node ty) with
          | (Readonly, Tfun fty) when not (get_ft_readonly_this fty) ->
            Errors.readonly_method_call (Tast.get_position e1) (get_pos ty)
          | _ -> ())
        | _ -> ()
      in
      check_readonly_call caller is_readonly;
      check_args caller args unpacked_arg;
      check_method_call caller

    method! on_method_ env m =
      let method_pos = fst m.m_name in
      let ret_pos = Typing_defs.get_pos (fst m.m_ret) in
      let this_ty =
        if m.m_readonly_this then
          Some (Readonly, method_pos)
        else
          Some (Mut, method_pos)
      in
      let new_ctx =
        {
          this_ty;
          ret_ty = Some (readonly_kind_to_rty m.m_readonly_ret, ret_pos);
          lenv = lenv_from_params m.m_params;
        }
      in
      ctx <- new_ctx;
      super#on_method_ env m

    method! on_fun_ env f =
      (* TODO: handle lambdas, which capture values. Const lambdas should make every value captured readonly. *)
      let ret_pos = Typing_defs.get_pos (fst f.f_ret) in
      let ret_ty = Some (readonly_kind_to_rty f.f_readonly_ret, ret_pos) in
      let new_ctx =
        { this_ty = None; ret_ty; lenv = lenv_from_params f.f_params }
      in
      ctx <- new_ctx;
      super#on_fun_ env f

    (* TODO: property accesses *)
    (* TODO: lambda expressions *)
    method! on_expr env e =
      match e with
      | (_, Binop (Ast_defs.Eq _, lval, rval)) ->
        self#assign lval rval;
        super#on_expr env e
      (* Readonly calls *)
      | (_, ReadonlyExpr (_, Call (caller, targs, args, unpacked_arg))) ->
        self#call ~is_readonly:true caller args unpacked_arg;
        (* Skip the recursive step into ReadonlyExpr to avoid erroring *)
        self#on_Call env caller targs args unpacked_arg
      (* Non readonly calls *)
      | (_, Call (caller, _, args, unpacked_arg)) ->
        self#call ~is_readonly:false caller args unpacked_arg;
        super#on_expr env e
      | _ -> super#on_expr env e

    method! on_stmt_ env s =
      (match s with
      | Return (Some e) ->
        (match ctx.ret_ty with
        | Some (ret_ty, pos) when not (subtype_rty (self#ty_expr e) ret_ty) ->
          Errors.readonly_mismatch
            "Invalid return"
            (Tast.get_position e)
            ~reason_sub:[(Tast.get_position e, "This expression is readonly")]
            ~reason_super:[(pos, "But this function does not return readonly.")]
        (* If we don't have a ret ty we're not in a function, must have errored somewhere else *)
        | _ -> ())
      | _ -> ());
      super#on_stmt_ env s
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_method_ env m = check#on_method_ env m

    method! at_fun_ env f = check#on_fun_ env f
  end
