(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Delta
open Typing_defs
open Typing_env_types
open Aast
module ETast = Tast
module Nast = Aast
module C = Typing_continuations
module Env = Typing_env
module Phase = Typing_phase
module Partial = Partial_provider

exception Cant_check
(** This happens, for example, when there are gotos *)

exception Not_implemented

let expect_ty_equal
    env ((pos, ty) : Pos.t * locl_ty) (expected_ty : locl_phase ty_) =
  let expected_ty = (Reason.none, expected_ty) in
  if not @@ ty_equal ~normalize_lists:true ty expected_ty then
    let actual_ty = Typing_print.debug env ty in
    let expected_ty = Typing_print.debug env expected_ty in
    Errors.unexpected_ty_in_tast pos ~actual_ty ~expected_ty

let refine ((_p, (_r, cond_ty)), cond_expr) _cond_is_true gamma =
  let is_refinement_fun fun_name =
    match fun_name with
    | "\\is_int"
    | "\\is_float"
    | "\\is_string"
    | "\\is_bool"
    | "\\is_array"
    | "\\is_vec"
    | "\\is_dict"
    | "\\is_keyset"
    | "\\is_resource" ->
      true
    | _ -> false
  in
  match cond_expr with
  | Call (Aast.Cnormal, (_, Id (_, id)), [], _, []) when is_refinement_fun id
    ->
    raise Not_implemented
  | Call
      (_, (_, Class_const ((_, CI (_, "\\Shapes")), (_, "keyExists"))), _, _, _)
    ->
    raise Not_implemented
  | Is _
  | As _
  | Binop _
  | Unop _
  | True
  | False ->
    raise Not_implemented
  | _ when cond_ty = Tprim Nast.Tbool -> gamma
  | _ -> raise Not_implemented

let check_assign (_lty, lvalue) ty gamma =
  match lvalue with
  | Lvar (_, lid) ->
    let gamma_updates = add_to_gamma lid ty empty_gamma in
    (gamma, gamma_updates)
  | _ -> raise Not_implemented

(* TODO for now check_expr is done with a visitor to deal more gracefully
 * with the unimplemented nodes: implemented nodes will be checked even if
 * they are not at the top-level. When all nodes are implemented, check_expr
 * might be simply redefined using the definition of on_expr. *)
(* This uses the iter visitor as an accumulator using a mutable variable
 * gamma as the accumulator, as showcased in the visitor documentation (fig. 2)
 * However, this results in a quite ugly programming style.
 * TODO write a proper accumulator visitor `accum` inheriting from iter and
 * using an `acc` mutable variable, then here inherit from `accum` instead of
 * `iter`. *)
let check_expr env (expr : ETast.expr) (gamma : gamma) : gamma =
  let expect_ty_equal = expect_ty_equal env in
  let expr_checker =
    object (self)
      inherit [_] Aast.iter as super

      val mutable gamma = gamma

      method gamma () = gamma

      method! on_expr env ((ty, expr) as texpr) =
        match expr with
        | True
        | False ->
          expect_ty_equal ty (Tprim Nast.Tbool)
        | Int _ -> expect_ty_equal ty (Tprim Nast.Tint)
        | Float _ -> expect_ty_equal ty (Tprim Nast.Tfloat)
        | String _ -> expect_ty_equal ty (Tprim Nast.Tstring)
        | Lvar (_, lid) ->
          let expected_ty =
            match lookup lid gamma.Typing_per_cont_env.local_types with
            | None -> Typing_defs.make_tany ()
            | Some (_p, (_r, ty)) -> ty
          in
          expect_ty_equal ty expected_ty
        | _ -> (* TODO *) super#on_expr env texpr

      method! on_Binop env bop expr1 ((ty2, _) as expr2) =
        match bop with
        | Ast_defs.Eq None ->
          let (gamma_, updates) = check_assign expr1 ty2 gamma in
          gamma <- gamma_;
          self#on_expr env expr2;
          gamma <- update_gamma gamma updates
        | Ast_defs.Eq _ -> raise Not_implemented
        | Ast_defs.Ampamp
        | Ast_defs.Barbar ->
          self#on_expr env expr1;
          let gamma_ = gamma in
          gamma <- refine expr1 (bop = Ast_defs.Ampamp) gamma;
          self#on_expr env expr2;
          gamma <- gamma_
        | _ -> (* TODO *) super#on_Binop env bop expr1 expr2

      method! on_Efun _env _fun _id_list = raise Not_implemented

      method! on_Lfun _env _fun _id_list = raise Not_implemented

      method! on_Class_const env class_id const_name =
        match (class_id, const_name) with
        | ((_, CI (_, "\\Shapes")), (_, "removeKey")) -> raise Not_implemented
        | _ -> super#on_Class_const env class_id const_name

      method! on_Eif _env _cond _e1 _e2 = raise Not_implemented

      method! on_Pipe _env _lid _e1 _e2 = raise Not_implemented

      method! on_As _env _e1 _ty = raise Not_implemented

      method! on_Callconv _env param_kind _expr =
        match param_kind with
        | Ast_defs.Pinout -> raise Not_implemented
    end
  in
  expr_checker#on_expr env expr;
  expr_checker#gamma ()

let rec check_stmt env (stmt : ETast.stmt) (gamma : gamma) : delta =
  let check_expr = check_expr env in
  match snd stmt with
  | Noop -> empty_delta_with_next_cont gamma
  | Expr expr ->
    let gamma = check_expr expr gamma in
    empty_delta_with_next_cont gamma
  | Fallthrough -> empty_delta_with_cont C.Fallthrough gamma
  | Break -> empty_delta_with_cont C.Break gamma
  (* TempBreak is caught as a naming error in naming.ml *)
  | TempBreak _ -> empty_delta_with_cont C.Break gamma
  | Continue -> empty_delta_with_cont C.Continue gamma
  (* TempContinue is caught as a naming error in naming.ml *)
  | TempContinue _ -> empty_delta_with_cont C.Continue gamma
  | Throw expr ->
    let gamma = check_expr expr gamma in
    empty_delta_with_cont C.Catch gamma
  | Return expropt ->
    let gamma =
      match expropt with
      | None -> gamma
      | Some expr ->
        let gamma = check_expr expr gamma in
        gamma
    in
    empty_delta_with_cont C.Exit gamma
  | If (cond, block1, block2) ->
    let gamma = check_expr cond gamma in
    let gamma1 = refine cond true gamma in
    let delta1 = check_block env block1 gamma1 in
    let gamma2 = refine cond false gamma in
    let delta2 = check_block env block2 gamma2 in
    union env delta1 delta2
  | Do _
  | While _
  | For _
  | Foreach _
  | Switch _
  | Try _
  | Using _
  | Def_inline _
  | Let _
  | Awaitall _ ->
    raise Not_implemented
  | Goto _
  | GotoLabel _
  | Block _
  | Markup _ ->
    raise Cant_check

and check_block env (block : ETast.block) gamma =
  let rec go block gamma delta =
    match block with
    | [] -> delta
    | stmt :: block ->
      let delta_s = check_stmt env stmt gamma in
      let delta = drop_cont C.Next delta in
      (* TODO there shouldn't be any need for 'env' because there shouldn't be
       * any type variable to resolve any more. *)
      let delta = union env delta delta_s in
      (match get_next_cont delta_s with
      | None -> delta
      | Some gamma -> go block gamma delta)
  in
  go block gamma (empty_delta_with_next_cont gamma)

let check_func_body env (body : ETast.func_body) gamma =
  if body.fb_annotation = Tast.HasUnsafeBlocks then
    raise Cant_check
  else
    let _ = check_block env body.fb_ast gamma in
    ()

(* TODO It's annoying to have to carry this giant 'env' everywhere. Can we
 * localize without it? *)
let localize env hint =
  match hint with
  | None -> failwith "There should be a hint in strict mode."
  | Some decl_ty ->
    let ty = Decl_hint.hint env.decl_env decl_ty in
    let (_env, ty) =
      Phase.localize ~ety_env:(Phase.env_with_self env) env ty
    in
    ty

let gamma_from_params env (params : ETast.fun_param list) =
  let add_param_to_gamma gamma param =
    if param.param_user_attributes <> [] then raise Not_implemented;
    let name = make_local_id param.param_name in
    let ty = localize env (hint_of_type_hint param.param_type_hint) in
    (* TODO can we avoid this? *)
    let reas_ty_to_pos_reas_ty ((r, _) as ty) = (Typing_reason.to_pos r, ty) in
    let ty = reas_ty_to_pos_reas_ty ty in
    add_to_gamma name ty gamma
  in
  List.fold ~init:empty_gamma ~f:add_param_to_gamma params

let check_fun env (f : ETast.fun_def) =
  if not (Partial.should_check_error f.f_mode 4291) then
    ()
  else
    let gamma = gamma_from_params env f.f_params in
    if
      f.f_tparams <> []
      || f.f_where_constraints <> []
      || f.f_variadic <> FVnonVariadic
    then
      raise Not_implemented
    else
      (* TODO check return type *)
      check_func_body env f.f_body gamma

let check_def env (def : ETast.def) =
  try
    match def with
    | Fun f -> check_fun env f
    | _ -> ()
  with
  | Cant_check ->
    Printf.printf "The TAST for this definition is not checkable.\n"
  | Not_implemented ->
    Printf.printf
      "The TAST for this definition contains nodes which are not yet implemented.\n"

let check_program env (program : ETast.program) =
  List.iter ~f:(check_def env) program

let check env tast =
  let (errors, ()) = Errors.do_ (fun () -> check_program env tast) in
  errors
