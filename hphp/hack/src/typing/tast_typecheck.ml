(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Delta
open Typing_defs
open Aast
module ETast = Tast
module Nast = Aast
module C = Typing_continuations
module Phase = Typing_phase

exception Cant_check

exception Not_implemented

let expect_ty_equal
    env (pos : Pos.t) (ty : locl_ty) (expected_ty : locl_phase ty_) =
  let expected_ty = mk (Reason.none, expected_ty) in
  if not @@ ty_equal ~normalize_lists:true ty expected_ty then
    let actual_ty = lazy (Typing_print.debug env ty) in
    let expected_ty = lazy (Typing_print.debug env expected_ty) in
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary @@ Primary.Unexpected_ty_in_tast { pos; actual_ty; expected_ty })

let refine (cond_ty, _, cond_expr) _cond_is_true gamma =
  let cond_ty_ = get_node cond_ty in
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
  | Call ((_, _, Id (_, id)), [], _, None) when is_refinement_fun id ->
    raise Not_implemented
  | Call
      ( (_, _, Class_const ((_, _, CI (_, "\\HH\\Shapes")), (_, "keyExists"))),
        _,
        _,
        _ ) ->
    raise Not_implemented
  | Is _
  | As _
  | Binop _
  | Unop _
  | True
  | False ->
    raise Not_implemented
  | _ when equal_locl_ty_ cond_ty_ (Tprim Nast.Tbool) -> gamma
  | _ -> raise Not_implemented

let check_assign (_lty, _, lvalue) p ty gamma =
  match lvalue with
  | Lvar (_, lid) ->
    let gamma_updates = add_to_gamma lid p ty empty_gamma in
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
let check_expr env ((_, p, _) as expr : ETast.expr) (gamma : gamma) : gamma =
  let expect_ty_equal = expect_ty_equal env p in
  let expr_checker =
    object (self)
      inherit [_] Aast.iter as super

      val mutable gamma = gamma

      method gamma () = gamma

      method! on_expr env ((ty, _, expr) as texpr) =
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
            | Some (_p, ty) -> get_node ty
          in
          expect_ty_equal ty expected_ty
        | _ -> (* TODO *) super#on_expr env texpr

      method! on_Binop
          env (Aast.{ bop; lhs = expr1; rhs = (ty2, p2, _) as expr2 } as binop)
          =
        match bop with
        | Ast_defs.Eq None ->
          let (gamma_, updates) = check_assign expr1 p2 ty2 gamma in
          gamma <- gamma_;
          self#on_expr env expr2;
          gamma <- update_gamma gamma updates
        | Ast_defs.Eq _ -> raise Not_implemented
        | Ast_defs.Ampamp
        | Ast_defs.Barbar ->
          self#on_expr env expr1;
          let gamma_ = gamma in
          gamma <- refine expr1 Ast_defs.(equal_bop bop Ampamp) gamma;
          self#on_expr env expr2;
          gamma <- gamma_
        | _ -> (* TODO *) super#on_Binop env binop

      method! on_Efun _env _ = raise Not_implemented

      method! on_Lfun _env _fun _id_list = raise Not_implemented

      method! on_Class_const env class_id const_name =
        match (class_id, const_name) with
        | ((_, _, CI (_, "\\HH\\Shapes")), (_, "removeKey")) ->
          raise Not_implemented
        | _ -> super#on_Class_const env class_id const_name

      method! on_Eif _env _cond _e1 _e2 = raise Not_implemented

      method! on_Pipe _env _lid _e1 _e2 = raise Not_implemented

      method! on_As _env _e1 _ty = raise Not_implemented
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
  | Continue -> empty_delta_with_cont C.Continue gamma
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
  | Yield_break
  | Do _
  | While _
  | For _
  | Foreach _
  | Switch _
  | Try _
  | Using _
  | Awaitall _ ->
    raise Not_implemented
  | Declare_local _
  | Block _
  | Markup _
  | AssertEnv _ ->
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
  let _ = check_block env body.fb_ast gamma in
  ()

(* TODO It's annoying to have to carry this giant 'env' everywhere. Can we
 * localize without it? *)
let localize env hint =
  match hint with
  | None -> failwith "There should be a hint in strict mode."
  | Some hint ->
    let pos = fst hint in
    let (_env, ty) =
      Phase.localize_hint_no_subst env ~ignore_errors:false hint
    in
    (pos, ty)

let gamma_from_params env (params : ETast.fun_param list) =
  let add_param_to_gamma gamma param =
    if not (List.is_empty param.param_user_attributes) then
      raise Not_implemented;
    let name = make_local_id param.param_name in
    let (pos, ty) = localize env @@ hint_of_type_hint param.param_type_hint in
    add_to_gamma name pos ty gamma
  in
  List.fold ~init:empty_gamma ~f:add_param_to_gamma params

let check_fun env (fd : ETast.fun_def) =
  let f = fd.fd_fun in
  let gamma = gamma_from_params env f.f_params in
  if
    (not (List.is_empty fd.fd_tparams))
    || not (List.is_empty fd.fd_where_constraints)
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
