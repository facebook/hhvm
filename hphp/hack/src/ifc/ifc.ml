(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_core
open Ifc_types
module Decl = Ifc_decl
module Env = Ifc_env
module Logic = Ifc_logic
module Pp = Ifc_pretty
module Lift = Ifc_lift
module Solver = Ifc_solver
module Utils = Ifc_utils
module A = Aast
module T = Typing_defs
module L = Logic.Infix
module K = Typing_cont_key
module TClass = Decl_provider.Class

exception FlowInference of string

let should_print ~user_mode ~phase =
  equal_mode user_mode Mdebug || equal_mode user_mode phase

let fail fmt = Format.kasprintf (fun s -> raise (FlowInference s)) fmt

(* A constraint accumulator that registers a subtyping
   requirement t1 <: t2 *)
let rec subtype ~pos t1 t2 acc =
  let subtype = subtype ~pos in
  match (t1, t2) with
  | (Tprim p1, Tprim p2)
  | (Tgeneric p1, Tgeneric p2) ->
    L.(p1 < p2) ~pos acc
  | (Ttuple tl1, Ttuple tl2) ->
    (match List.zip tl1 tl2 with
    | Some zip ->
      List.fold zip ~init:acc ~f:(fun acc (t1, t2) -> subtype t1 t2 acc)
    | None -> fail "incompatible tuple types")
  | (Tclass cl1, Tclass cl2) ->
    (* Nominal subtyping records flows in properties (through the lump policy)
     * common to both classes. The typechecker ensures that the subtyping
     * relation holds, so we do not need safety checks.
     *
     * We do not need constraints for policied properties as their policies are
     * known statically. Hence, the only flow constraint we have can have due
     * to subtyping of these is the trivial one of the form A < A.
     *)
    acc
    (* Invariant in lump policy *)
    |> L.(cl1.c_lump = cl2.c_lump) ~pos
    (* Covariant in class policy *)
    |> L.(cl1.c_self < cl2.c_self) ~pos
  | (Tfun f1, Tfun f2) ->
    let zipped_args =
      match List.zip f1.f_args f2.f_args with
      | Some zip -> zip
      | None -> failwith "functions have different number of arguments"
    in
    (* Contravariant in argument types *)
    List.fold ~f:(fun acc (t1, t2) -> subtype t2 t1 acc) ~init:acc zipped_args
    (* Contravariant in PC *)
    |> L.(f2.f_pc < f1.f_pc) ~pos
    (* Covariant in its own identity *)
    |> L.(f1.f_self < f2.f_self) ~pos
    (* Covariant in return type and exceptions *)
    |> L.(subtype f1.f_ret f2.f_ret && subtype f1.f_exn f2.f_exn)
  | (Tunion tl, _) ->
    List.fold tl ~init:acc ~f:(fun acc t1 -> subtype t1 t2 acc)
  | (pty1, pty2) ->
    fail "unhandled subtyping query for %a <: %a" Pp.ptype pty1 Pp.ptype pty2

(* A constraint accumulator that registers that t1 = t2 *)
and equivalent ~pos t1 t2 acc =
  let subtype = subtype ~pos in
  subtype t1 t2 (subtype t2 t1 acc)

(* Generates a fresh sub/super policy of the argument polic *)
let adjust_policy ?(prefix = "weak") ~pos ~adjustment renv env policy =
  match (adjustment, policy) with
  | (Astrengthen, Pbot _)
  | (Aweaken, Ptop _) ->
    (env, policy)
  | (Astrengthen, _) ->
    let new_policy = Env.new_policy_var renv prefix in
    let prop = L.(new_policy < policy) ~pos in
    (Env.acc env prop, new_policy)
  | (Aweaken, _) ->
    let new_policy = Env.new_policy_var renv prefix in
    let prop = L.(policy < new_policy) ~pos in
    (Env.acc env prop, new_policy)

(* Generates a fresh sub/super ptype of the argument ptype *)
let adjust_ptype ?prefix ~pos ~adjustment renv env ty =
  let adjust_policy = adjust_policy ?prefix ~pos renv in
  let flip = function
    | Astrengthen -> Aweaken
    | Aweaken -> Astrengthen
  in
  let rec freshen adjustment env ty =
    let freshen_cov = freshen adjustment in
    let freshen_con = freshen @@ flip adjustment in
    let freshen_pol_cov = adjust_policy ~adjustment in
    let freshen_pol_con = adjust_policy ~adjustment:(flip adjustment) in
    let simple_freshen f policy =
      let (env, p) = freshen_pol_cov env policy in
      (env, f p)
    in
    let on_list mk tl =
      let (env, tl') = List.map_env env tl ~f:freshen_cov in
      (env, mk tl')
    in
    match ty with
    | Tprim policy -> simple_freshen (fun p -> Tprim p) policy
    | Tgeneric policy -> simple_freshen (fun p -> Tgeneric p) policy
    | Ttuple tl -> on_list (fun l -> Ttuple l) tl
    | Tunion tl -> on_list (fun l -> Tunion l) tl
    | Tinter tl -> on_list (fun l -> Tinter l) tl
    | Tclass class_ ->
      let (env, super_pol) = freshen_pol_cov env class_.c_self in
      (env, Tclass { class_ with c_self = super_pol })
    | Tfun fun_ ->
      let (env, f_pc) = freshen_pol_con env fun_.f_pc in
      let (env, f_self) = freshen_pol_cov env fun_.f_self in
      let (env, f_args) = List.map_env env ~f:freshen_con fun_.f_args in
      let (env, f_ret) = freshen_cov env fun_.f_ret in
      let (env, f_exn) = freshen_cov env fun_.f_exn in
      (env, Tfun { f_pc; f_self; f_args; f_ret; f_exn })
  in
  freshen adjustment env ty

let rec add_dependencies ~pos pl t acc =
  match t with
  | Tinter [] ->
    (* The policy p flows into a value of type mixed.
       Here we choose that mixed will be public; we
       could choose a different default policy or
       have mixed carry a policy (preferable). *)
    L.(pl <* [Pbot PosSet.empty]) ~pos acc
  | Tprim pol
  | Tgeneric pol
  (* For classes and functions, we only add a dependency to the self policy and
     not to its properties *)
  | Tclass { c_self = pol; _ }
  | Tfun { f_self = pol; _ } ->
    L.(pl <* [pol]) ~pos acc
  | Tunion tl
  | Ttuple tl
  | Tinter tl ->
    let f acc t = add_dependencies ~pos pl t acc in
    List.fold tl ~init:acc ~f

let policy_join ~pos renv env ?(prefix = "join") p1 p2 =
  match Logic.policy_join p1 p2 with
  | Some p -> (env, p)
  | None ->
    (match (p1, p2) with
    | (Pfree_var (v1, s1), Pfree_var (v2, s2))
      when equal_policy_var v1 v2 && Scope.equal s1 s2 ->
      (env, p1)
    | _ ->
      let pv = Env.new_policy_var renv prefix in
      let env = Env.acc env L.((p1 < pv) ~pos && (p2 < pv) ~pos) in
      (env, pv))

let mk_union t1 t2 =
  let (l1, l2) =
    match (t1, t2) with
    | (Tunion u1, Tunion u2) -> (u1, u2)
    | (Tunion u, t) -> ([t], u)
    | (t, Tunion u) -> ([t], u)
    | _ -> ([t1], [t2])
  in
  let merge_type_lists l1 l2 =
    let f t = not (List.exists l2 ~f:(phys_equal t)) in
    List.filter ~f l1 @ l2
  in
  match merge_type_lists l1 l2 with
  | [t] -> t
  | u -> Tunion u

(* Uses a Hack-inferred type to update the flow type of a local
   variable *)
let refresh_lvar_type ~pos renv env lid (ety : T.locl_ty) =
  (* TODO(T68306543): make this faster when ety is the skeleton
     of the type we already have for lid *)
  let is_simple pty =
    match pty with
    | Tunion _ -> false
    | _ -> true
  in
  let pty = Env.get_local_type env lid in
  if is_simple pty then
    (* if the type is already simple, do not refresh it with
       what Hack found *)
    (env, pty)
  else
    let prefix = Local_id.to_string lid in
    let new_pty = Lift.ty renv ety ~prefix in
    let env = Env.acc env (subtype ~pos pty new_pty) in
    let env = Env.set_local_type env lid new_pty in
    (env, new_pty)

let add_params renv =
  let add_param env p =
    let prefix = p.A.param_name in
    let pty = Lift.ty ~prefix renv (fst p.A.param_type_hint) in
    let lid = Local_id.make_unscoped p.A.param_name in
    let env = Env.set_local_type env lid pty in
    (env, pty)
  in
  List.map_env ~f:add_param

let binop ~pos renv env ty1 ty2 =
  match (ty1, ty2) with
  | (Tprim p1, Tprim p2) ->
    let (env, pj) = policy_join ~pos renv env ~prefix:"bop" p1 p2 in
    (env, Tprim pj)
  | _ -> fail "unexpected Binop types"

let receiver_of_obj_get obj_ptype property =
  match obj_ptype with
  | Tclass class_ -> class_
  | _ -> fail "couldn't find a class for the property '%s'" property

(* We generate a ptype out of the property type and fill it with either the
 * purpose of property or the lump policy of some object root. *)
let property_ptype renv obj_ptype property property_ty =
  let class_ = receiver_of_obj_get obj_ptype property in
  let prop_pol =
    match Decl.property_policy renv.re_decl class_.c_name property with
    | Some policy -> policy
    | None -> class_.c_lump
  in
  Lift.ty ~prefix:property ~lump:prop_pol renv property_ty

let throw ~pos renv env exn_ty =
  let union env t1 t2 = (env, mk_union t1 t2) in
  let env = Env.merge_conts_into ~union env [K.Next] K.Catch in
  let lpc = Env.get_lpc_policy env K.Next in
  Env.acc
    env
    L.(
      subtype ~pos exn_ty renv.re_exn
      && add_dependencies ~pos (PCSet.elements lpc) renv.re_exn)

let call ~pos renv env call_type that_pty_opt args_pty ret_ty =
  let (callee, ret_pty) =
    let name =
      match call_type with
      | Cglobal callable_name -> callable_name
      | Clocal _ -> "anonymous"
    in
    let ret_pty = Lift.ty ~prefix:(name ^ "_ret") renv ret_ty in
    let callee = Env.new_policy_var renv name in
    (callee, ret_pty)
  in
  let callee_exn_policy = Env.new_policy_var renv "exn" in
  let callee_exn =
    Lift.class_ty ~lump:callee_exn_policy renv Decl.exception_id
  in
  (* The PC of the function being called depends on the join of the current
   * PC dependencies, as well as the function's own self policy *)
  let (env, pc_joined) =
    let join pc' (env, pc) =
      policy_join ~pos renv env ~prefix:"pcjoin" pc pc'
    in
    PCSet.fold join (Env.get_gpc_policy renv env K.Next) (env, callee)
  in
  let hole_ty =
    {
      f_pc = pc_joined;
      f_self = callee;
      f_args = args_pty;
      f_ret = ret_pty;
      f_exn = callee_exn;
    }
  in
  let env =
    match call_type with
    | Clocal fty -> Env.acc env @@ subtype ~pos (Tfun fty) (Tfun hole_ty)
    | Cglobal callable_name ->
      let fp =
        { fp_name = callable_name; fp_this = that_pty_opt; fp_type = hole_ty }
      in
      let (env, call_constraint) =
        match SMap.find_opt callable_name renv.re_decl.de_fun with
        | Some { fd_kind = FDGovernedBy policy; fd_args } ->
          let scheme = Decl.make_callable_scheme renv policy fp fd_args in
          let prop =
            (* because cipp_scheme is created after fp they cannot
               mismatch and call_constraint will not fail *)
            Option.value_exn (Solver.call_constraint ~subtype ~pos fp scheme)
          in
          (env, prop)
        | Some { fd_kind = FDInferFlows; _ } ->
          let env = Env.add_dep env callable_name in
          (env, Chole (pos, fp))
        | None -> fail "unknown function '%s'" callable_name
      in
      Env.acc env (fun acc -> call_constraint :: acc)
  in
  let env = Env.acc env @@ add_dependencies ~pos [callee] ret_pty in
  (* Any function call may throw, so we need to update the current PC and
   * exception dependencies based on the callee's exception policy
   *)
  let env = Env.push_pc env K.Next callee_exn_policy in
  let env = throw ~pos renv env callee_exn in
  (env, ret_pty)

let vec_element_pty renv vec_ty vec_pty =
  let lump =
    match vec_pty with
    | Tclass cls when String.equal cls.c_name Decl.vec_id -> cls.c_lump
    | _ -> fail "expected a vector"
  in
  let rec element_pty vec_ty =
    match T.get_node vec_ty with
    | T.Tclass ((_, c_name), _, [element_ty])
      when String.equal c_name Decl.vec_id ->
      Lift.ty ~lump renv element_ty
    | T.Tvar id -> element_pty @@ Lift.expand_var renv id
    | _ ->
      fail
        "expected one type parameter from a vector object, but got %s"
        (Pp_type.show_ty () vec_ty)
  in
  element_pty vec_ty

(* Finds what we flow into in an assignment. Uses the TAST invariant that
 * the lhs' type is the type after the assignment occurred. *)
let rec flux_target ~pos renv env ((_, lhs_ty), lhs_exp) =
  let expr = expr ~pos renv in
  match lhs_exp with
  | A.Lvar (_, lid) ->
    let prefix = Local_id.to_string lid in
    let local_pty = Lift.ty ~prefix renv lhs_ty in
    let env = Env.set_local_type env lid local_pty in
    (env, local_pty, Env.get_lpc_policy env K.Next)
  | A.Obj_get (obj, (_, A.Id (_, property)), _) ->
    let (env, obj_pty) = expr env obj in
    let obj_pol = (receiver_of_obj_get obj_pty property).c_self in
    let prop_pty = property_ptype renv obj_pty property lhs_ty in
    let env = Env.acc env (add_dependencies ~pos [obj_pol] prop_pty) in
    (env, prop_pty, Env.get_gpc_policy renv env K.Next)
  | A.Array_get ((((_, vec_ty), _) as vec), ix_opt) ->
    (* Copy-on-Write handling of Array_get LHS in an assignment. When there is
     * an assignmnet lhs[ix] = rhs or lhs[] = rhs, we
     * 1. evaluate ix in case it has side-effects
     * 2. (a) retrieve the original flux type for the vector
     *    (b) use flux_target recursively on the vector to generate a new flux
     *        type and retrieve info. from the mutation root
     *    (c) make the original flux type flow into the new one to achieve CoW
     * 3. record the flow due to length increase when ix is not present. (not
     *    implemented, TODO: T68269878)
     * 4. access and return the element parameter of the vector
     *)
    let env =
      match ix_opt with
      | Some ix -> fst @@ expr env ix
      | None -> env
    in
    let (env, old_vec_pty) = expr env vec in
    let (env, vec_pty, pc) = flux_target ~pos renv env vec in
    let env = Env.acc env @@ subtype ~pos old_vec_pty vec_pty in
    let element_pty = vec_element_pty renv vec_ty vec_pty in
    (* Even though the runtime updates the entire vector, we treat
     * the mutation as if it were happening on a single element.
     * That is sound because, after the mutation, changes can only
     * be observed via the updated element.
     *)
    (env, element_pty, pc)
  | _ -> fail "unhandled flux target (lvalue)"

and assign ~pos renv env op lhs_exp rhs_exp =
  let expr = expr ~pos renv in
  let (env, rhs_pty) =
    if Option.is_none op then
      expr env rhs_exp
    else
      (* increment-like operations (e.g., $a += $b) *)
      let (env, lhs_pty) = expr env lhs_exp in
      let (env, rhs_pty) = expr env rhs_exp in
      binop ~pos renv env lhs_pty rhs_pty
  in
  let (env, lhs_pty, pc) = flux_target ~pos renv env lhs_exp in
  let env = Env.acc env (subtype ~pos rhs_pty lhs_pty) in
  let env = Env.acc env (add_dependencies ~pos (PCSet.elements pc) lhs_pty) in
  (env, lhs_pty)

(* Generate flow constraints for an expression *)
and expr ~pos renv env (((epos, ety), e) : Tast.expr) =
  let expr = expr ~pos renv in
  match e with
  | A.True
  | A.False
  | A.Int _
  | A.Float _
  | A.String _ ->
    (* literals are public *)
    (env, Tprim (Pbot (PosSet.singleton epos)))
  | A.Binop (Ast_defs.Eq op, e1, e2) -> assign ~pos renv env op e1 e2
  | A.Binop (_, e1, e2) ->
    let (env, ty1) = expr env e1 in
    let (env, ty2) = expr env e2 in
    binop ~pos renv env ty1 ty2
  | A.Lvar (_pos, lid) -> refresh_lvar_type ~pos renv env lid ety
  | A.Obj_get (obj, (_, A.Id (_, property)), _) ->
    let (env, obj_ptype) = expr env obj in
    let prop_pty = property_ptype renv obj_ptype property ety in
    let prefix = "." ^ property in
    let (env, super_pty) =
      adjust_ptype ~pos ~prefix ~adjustment:Aweaken renv env prop_pty
    in
    let obj_pol = (receiver_of_obj_get obj_ptype property).c_self in
    let env = Env.acc env (add_dependencies ~pos [obj_pol] super_pty) in
    (env, super_pty)
  | A.This ->
    (match renv.re_this with
    | Some ptype -> (env, ptype)
    | None -> fail "encountered $this outside of a class context")
  | A.ET_Splice e
  | A.ExpressionTree (_, e)
  | A.BracedExpr e ->
    expr env e
  (* TODO(T68414656): Support calls with type arguments *)
  | A.Call (_call_type, e, _type_args, args, _extra_args) ->
    let (env, args_pty) = List.map_env ~f:expr env args in
    let call env call_type this_pty =
      call ~pos renv env call_type this_pty args_pty ety
    in
    begin
      match e with
      | (_, A.Id (_, name)) ->
        let call_id = Decl.make_callable_name None name in
        call env (Cglobal call_id) None
      | (_, A.Obj_get (obj, (_, A.Id (_, meth_name)), _)) ->
        let (env, obj_pty) = expr env obj in
        begin
          match obj_pty with
          | Tclass { c_name; _ } ->
            let call_id = Decl.make_callable_name (Some c_name) meth_name in
            call env (Cglobal call_id) (Some obj_pty)
          | _ -> fail "unhandled method call on %a" Pp.ptype obj_pty
        end
      | (_, A.Class_const (((_, ty), cid), (_, meth_name))) ->
        let env =
          match cid with
          | A.CIexpr e -> fst @@ expr env e
          | A.CIstatic ->
            (* TODO(T72024862): Handle late static binding *)
            fail "late static binding is not supported"
          | _ -> env
        in
        let rec find_class_name ty =
          match T.get_node ty with
          | T.Tdependent (T.DTthis, ty) -> find_class_name ty
          | T.Tclass ((_, class_name), _, _) -> class_name
          | _ -> fail "unhandled method call on a non-class"
        in
        let class_name = find_class_name ty in
        let call_id = Decl.make_callable_name (Some class_name) meth_name in
        let this_pty =
          if String.equal meth_name Decl.construct_id then begin
            (* The only legal class id is `parent` which is invoked as if it
               is invoked on the object being constructed, hence it uses the same
               `$this` as the caller. *)
            assert (Option.is_some renv.re_this);
            renv.re_this
          end else
            None
        in
        call env (Cglobal call_id) this_pty
      | _ ->
        let (env, func_ty) = expr env e in
        let fty =
          match func_ty with
          | Tfun fty -> fty
          | _ -> failwith "calling something that is not a function"
        in
        call env (Clocal fty) None
    end
  | A.ValCollection (A.Vec, _, exprs) ->
    (* We require each collection element to be a subtype of the vector
     * element parameter. *)
    let vec_pty = Lift.ty ~prefix:"vec" renv ety in
    let element_pty = vec_element_pty renv ety vec_pty in
    let mk_element_subtype env exp =
      let (env, pty) = expr env exp in
      Env.acc env (subtype ~pos pty element_pty)
    in
    let env = List.fold ~f:mk_element_subtype ~init:env exprs in
    (env, vec_pty)
  | A.Array_get ((((_, vec_ty), _) as vec), ix_opt) ->
    (* Return the type parameter corresponding to the vector element type. *)
    let env =
      (* Evaluate the index in case it has side-effects. *)
      match ix_opt with
      | Some ix -> fst @@ expr env ix
      | None -> fail "cannot have an empty index when reading"
    in
    let (env, vec_pty) = expr env vec in
    let element_pty = vec_element_pty renv vec_ty vec_pty in
    (env, element_pty)
  (* TODO(T70139741): support variadic functions and constructors
   * TODO(T70139893): support classes with type parameters
   *)
  | A.New (((_, lty), cid), _targs, args, _extra_args, _) ->
    let (env, args_pty) = List.map_env ~f:expr env args in
    let env =
      match cid with
      | A.CIexpr e -> fst @@ expr env e
      | A.CIstatic ->
        (* TODO(T72024862): Handle late static binding *)
        fail "late static binding is not supported"
      | _ -> env
    in
    let obj_pty = Lift.ty renv lty in
    begin
      match obj_pty with
      | Tclass { c_name; _ } ->
        let call_id = Decl.make_callable_name (Some c_name) Decl.construct_id in
        let lty = T.mk (Typing_reason.Rnone, T.Tprim A.Tvoid) in
        let (env, _) =
          call ~pos renv env (Cglobal call_id) (Some obj_pty) args_pty lty
        in
        let pty = Lift.ty ~prefix:"constr" renv ety in
        (env, pty)
      | _ -> fail "unhandled method call on %a" Pp.ptype obj_pty
    end
  | A.Efun (fun_, captured_ids)
  | A.Lfun (fun_, captured_ids) ->
    (* Stash the cenv so it can be restored later *)
    let pre_cenv = Env.get_cenv env in
    (* Drop all conts except for Next and drop the local PC because we are
     * entering a new scope of execution. Freshen all the local variables since
     * their changes are not visible outside of the lambda's scope
     *)
    let env = Env.filter_conts env (K.equal K.Next) in
    let env = Env.set_pc env K.Next PCSet.empty in
    let env =
      let freshen = adjust_ptype ~pos ~adjustment:Aweaken in
      Env.freshen_cenv ~freshen renv env captured_ids
    in

    let pc = Env.new_policy_var renv "pc" in
    let self = Env.new_policy_var renv "lambda" in
    let (env, ptys) = add_params renv env fun_.A.f_params in
    let exn = Lift.class_ty renv Decl.exception_id in
    let ret = Lift.ty ~prefix:"ret" renv (fst fun_.A.f_ret) in
    let renv = { renv with re_ret = ret; re_exn = exn; re_gpc = pc } in

    let env = block renv env fun_.A.f_body.A.fb_ast in

    (* Restore conts now that we exit the lambda's scope *)
    let env = Env.set_cenv env pre_cenv in
    let ty =
      Tfun { f_pc = pc; f_self = self; f_args = ptys; f_ret = ret; f_exn = exn }
    in
    (env, ty)
  (* --- expressions below are not yet supported *)
  | A.Darray (_, _)
  | A.Varray (_, _)
  | A.Shape _
  | A.ValCollection (_, _, _)
  | A.KeyValCollection (_, _, _)
  | A.Null
  | A.Omitted
  | A.Id _
  | A.Dollardollar _
  | A.Clone _
  | A.Obj_get (_, _, _)
  | A.Class_get (_, _)
  | A.Class_const (_, _)
  | A.FunctionPointer (_, _)
  | A.String2 _
  | A.PrefixedString (_, _)
  | A.Yield _
  | A.Yield_break
  | A.Await _
  | A.Suspend _
  | A.List _
  | A.Expr_list _
  | A.Cast (_, _)
  | A.Unop (_, _)
  | A.Pipe (_, _, _)
  | A.Eif (_, _, _)
  | A.Is (_, _)
  | A.As (_, _, _)
  | A.Record (_, _)
  | A.Xml (_, _, _)
  | A.Callconv (_, _)
  | A.Import (_, _)
  | A.Collection (_, _, _)
  | A.ParenthesizedExpr _
  | A.Lplaceholder _
  | A.Fun_id _
  | A.Method_id (_, _)
  | A.Method_caller (_, _)
  | A.Smethod_id (_, _)
  | A.Pair _
  | A.Assert _
  | A.PU_atom _
  | A.PU_identifier (_, _, _)
  | A.Any ->
    fail "expr"

and stmt renv env ((pos, s) : Tast.stmt) =
  let expr = expr renv in
  match s with
  | A.Expr e ->
    let (env, _ty) = expr ~pos env e in
    env
  | A.If ((((pos, _), _) as e), b1, b2) ->
    let (env, ety) = expr ~pos env e in
    (* Stash the PC so they can be restored after the if *)
    let pc = Env.get_lpc_policy env K.Next in
    let env =
      let epol =
        match ety with
        | Tprim p -> p
        | _ -> fail "condition expression must be of type bool"
      in
      Env.push_pc env K.Next epol
    in
    let cenv = Env.get_cenv env in
    let env = block renv (Env.set_cenv env cenv) b1 in
    let cenv1 = Env.get_cenv env in
    let env = block renv (Env.set_cenv env cenv) b2 in
    let cenv2 = Env.get_cenv env in
    let union _env t1 t2 = (env, mk_union t1 t2) in
    let env = Env.merge_and_set_cenv ~union env cenv1 cenv2 in
    (* Restore the program counter from before the IF *)
    Env.set_pc env K.Next pc
  | A.Return e ->
    let union env t1 t2 = (env, mk_union t1 t2) in
    let env = Env.merge_conts_into ~union env [K.Next] K.Exit in
    begin
      match e with
      | None -> env
      | Some e ->
        let (env, te) = expr ~pos env e in
        (* to account for enclosing conditionals, make the return
          type depend on the local pc *)
        let lpc = Env.get_lpc_policy env K.Next in
        Env.acc
          env
          L.(
            add_dependencies ~pos (PCSet.elements lpc) renv.re_ret
            && subtype ~pos te renv.re_ret)
    end
  | A.Throw e ->
    let (env, exn_ty) = expr ~pos env e in
    throw ~pos renv env exn_ty
  | A.Try (try_blk, [((_, exn), (_, exn_var), catch_blk)], []) ->
    (* NOTE: for now we only support try with a single catch block and no finally
     * block. Only \Exception is allowed to be caught
     *)
    let union env t1 t2 = (env, mk_union t1 t2) in
    let base_cenv = Env.get_cenv env in
    let base_pc = Env.get_lpc_policy env K.Next in
    let env = Env.drop_conts env [K.Catch] in

    (* Create a fresh exception for the try block since none of the outer
     * exceptions are catchable inside the block
     *)
    let try_renv =
      let fresh_exn = Lift.class_ty renv Decl.exception_id in
      { renv with re_exn = fresh_exn }
    in
    let env = block try_renv env try_blk in

    if not @@ String.equal exn Decl.exception_id then
      fail "catch is only supported for \\Exception, got '%s'" exn;
    (* The catch block should begin with the catch continuation replacing Next *)
    let env = Env.drop_conts env [K.Next] in
    let env = Env.move_conts_into ~union env [K.Catch] K.Next in
    let env = Env.set_local_type env exn_var try_renv.re_exn in
    let env = block renv env catch_blk in

    let env = Env.merge_conts_from ~union env base_cenv [K.Catch] in
    Env.set_pc env K.Next base_pc
  | _ -> env

and block renv env (blk : Tast.block) =
  let seq env s =
    let env = stmt renv env s in
    Env.merge_pcs_into env [K.Exit; K.Catch] K.Next
  in
  List.fold_left ~f:seq ~init:env blk

(* Returns the list of free policy variables in a policied type *)
let free_pvars =
  let on_policy acc p =
    match p with
    | Pfree_var (v, s) ->
      acc := VarSet.add (v, s) !acc;
      p
    | _ -> p
  in
  let rec iter acc p = Ifc_mapper.ptype (iter acc) (on_policy acc) p in
  fun pty ->
    let acc = ref VarSet.empty in
    let _ = iter acc pty in
    !acc

(* Checks that two type schemes are in a subtyping relationship. The
 * skeletons of the two input type schemes are expected to be same.
 * A list of invalid flows is returned, if the list is empty the
 * type schemes are in a subtyping relationship. *)
let check_subtype_scheme ~pos sub_scheme sup_scheme : pos_flow list =
  let (Fscheme (sub_scope, sub_proto, sub_prop)) = sub_scheme in
  let (Fscheme (sup_scope, sup_proto, sup_prop)) = sup_scheme in
  assert (not (Scope.equal sub_scope sup_scope));
  (* To compare them, we need the two constraints to use the same set
     of free variables. For example, if sub_scheme and sup_scheme are
       - ((function():int{p1}), Csub) and,
       - ((function():int{p2}), Csup)
     we rename p2 into p1 by conjoining p2 = p1 to Csup and
     quantifying p2 away. *)
  let sup_props =
    let accum = ref [sup_prop] in
    let rec equate pt1 pt2 =
      let eqpol p1 p2 = accum := L.(p1 = p2) ~pos !accum in
      Ifc_mapper.iter_ptype2 equate eqpol pt1 pt2
    in
    equate (Tfun sub_proto.fp_type) (Tfun sup_proto.fp_type);
    begin
      match (sub_proto.fp_this, sup_proto.fp_this) with
      | (Some sub_this_ty, Some sup_this_ty) -> equate sub_this_ty sup_this_ty
      | (None, None) -> ()
      | _ -> invalid_arg "method/function mismatch"
    end;
    !accum
  in
  let sub_vars =
    let fp_vars = free_pvars (Tfun sub_proto.fp_type) in
    match sub_proto.fp_this with
    | Some sub_this_ty -> VarSet.union fp_vars (free_pvars sub_this_ty)
    | None -> fp_vars
  in
  let pred v = not @@ VarSet.mem v sub_vars in
  let sup_lattice =
    Logic.conjoin sup_props
    |> Logic.quantify ~pred ~quant:Qexists
    |> Logic.simplify
    |> Logic.flatten_prop
    |> Ifc_security_lattice.transitive_closure
  in
  let sub_prop =
    sub_prop |> Logic.quantify ~pred ~quant:Qexists |> Logic.simplify
  in
  (* Format.printf "%s@." sub_proto.fp_name;
   * Format.printf "LATTICE: %a@." Pp.security_lattice sup_lattice;
   * Format.printf "PROP: %a@." Pp.prop sub_prop; *)
  Logic.entailment_violations sup_lattice sub_prop

let analyse_callable
    ?class_name
    ~pos
    ~opts
    ~decl_env
    ~is_static
    ~saved_env
    name
    params
    body
    return =
  try
    (* Setup the read-only environment *)
    let scope = Scope.alloc () in
    let renv = Env.new_renv scope decl_env saved_env in

    let global_pc = Env.new_policy_var renv "pc" in
    let exn = Lift.class_ty renv Decl.exception_id in

    (* Here, we ignore the type parameters of this because at the moment we
     * lack Tgeneric policy type. This will be fixed (T68414656) in the future.
     *)
    let this_ty =
      match class_name with
      | Some cname when not is_static -> Some (Lift.class_ty renv cname)
      | _ -> None
    in
    let ret_ty = Lift.ty ~prefix:"ret" renv return in
    let renv = Env.prep_renv renv this_ty ret_ty global_pc exn in

    (* Initialise the mutable environment *)
    let env = Env.new_env in
    let (env, param_tys) = add_params renv env params in

    (* Run the analysis *)
    let beg_env = env in
    let env = block renv env body.A.fb_ast in
    let end_env = env in

    (* Display the analysis results *)
    if should_print opts.opt_mode Manalyse then begin
      Format.printf "Analyzing %s:@." name;
      Format.printf "%a@." Pp.renv renv;
      Format.printf "* Params:@,  %a@." Pp.locals beg_env;
      Format.printf "* Final environment:@,  %a@." Pp.env end_env;
      Format.printf "@."
    end;

    let callable_name = Decl.make_callable_name class_name name in
    let f_self = Env.new_policy_var renv callable_name in

    let proto =
      {
        fp_name = callable_name;
        fp_this = this_ty;
        fp_type =
          {
            f_pc = global_pc;
            f_self;
            f_args = param_tys;
            f_ret = ret_ty;
            f_exn = exn;
          };
      }
    in

    let entailment =
      match SMap.find_opt callable_name decl_env.de_fun with
      | Some { fd_kind = FDGovernedBy policy; fd_args } ->
        let scheme = Decl.make_callable_scheme renv policy proto fd_args in
        fun prop ->
          let fun_scheme = Fscheme (scope, proto, prop) in
          check_subtype_scheme ~pos fun_scheme scheme
      | _ -> const []
    in

    (* Return the results *)
    let res =
      {
        res_span = pos;
        res_proto = proto;
        res_scope = scope;
        res_constraint = Logic.conjoin env.e_acc;
        res_deps = env.e_deps;
        res_entailment = entailment;
      }
    in
    Some res
  with FlowInference s ->
    Format.printf "Analyzing %s:@.  Failure: %s@.@." name s;
    None

let walk_tast opts decl_env =
  let def = function
    | A.Fun
        {
          A.f_name = (_, name);
          f_annotation = saved_env;
          f_params = params;
          f_body = body;
          f_ret = (return, _);
          f_span = pos;
          _;
        } ->
      let is_static = false in
      let callable_res =
        analyse_callable
          ~opts
          ~pos
          ~decl_env
          ~is_static
          ~saved_env
          name
          params
          body
          return
      in
      Option.map ~f:(fun x -> [x]) callable_res
    | A.Class { A.c_name = (_, class_name); c_methods = methods; _ } ->
      let handle_method
          {
            A.m_name = (_, name);
            m_annotation = saved_env;
            m_params = params;
            m_body = body;
            m_ret = (return, _);
            m_span = pos;
            m_static = is_static;
            _;
          } =
        analyse_callable
          ~opts
          ~class_name
          ~pos
          ~decl_env
          ~is_static
          ~saved_env
          name
          params
          body
          return
      in
      Some (List.filter_map ~f:handle_method methods)
    | _ -> None
  in
  (fun tast -> List.concat (List.filter_map ~f:def tast))

let check opts tast =
  (* Declaration phase *)
  let decl_env = Decl.collect_sigs tast in
  if should_print ~user_mode:opts.opt_mode ~phase:Mdecl then
    Format.printf "%a@." Pp.decl_env decl_env;

  (* Flow analysis phase *)
  let results = walk_tast opts decl_env tast in

  (* Solver phase *)
  let results =
    try Solver.global_exn ~subtype results with
    | Solver.Error Solver.RecursiveCycle ->
      fail "solver error: cyclic call graph"
    | Solver.Error (Solver.MissingResults callable) ->
      fail "solver error: missing results for callable '%s'" callable
    | Solver.Error (Solver.InvalidCall (caller, callee)) ->
      fail "solver error: invalid call to '%s' in '%s'" callee caller
  in

  let simplify result =
    let pred = const true in
    ( result,
      result.res_entailment result.res_constraint,
      Logic.simplify
      @@ Logic.quantify ~pred ~quant:Qexists result.res_constraint )
  in
  let simplified_results = SMap.map simplify results in

  let log_solver name (result, _, simplified) =
    Format.printf "@[<v>";
    Format.printf "Flow constraints for %s:@.  @[<v>" name;
    Format.printf "@,@[<hov>Simplified:@ @[<hov>%a@]@]" Pp.prop simplified;
    let raw = result.res_constraint in
    Format.printf "@,@[<hov>Raw:@ @[<hov>%a@]@]" Pp.prop raw;
    Format.printf "@]";
    Format.printf "@]\n\n"
  in
  if should_print ~user_mode:opts.opt_mode ~phase:Msolve then
    SMap.iter log_solver simplified_results;

  (* Checking phase *)
  let check_valid_flow _ (result, implicit, simple) =
    let simple_illegal_flows =
      Logic.entailment_violations opts.opt_security_lattice simple
    in
    let to_err node =
      (PosSet.elements (pos_of node), Format.asprintf "%a" Pp.policy node)
    in
    let illegal_information_flow (poss, source, sink) =
      (* Separate error positions that are not in the result and filter out
         unknown positions *)
      let (primary_poss, other_poss) =
        PosSet.filter (fun p -> not @@ Pos.equal Pos.none p) poss
        |> PosSet.elements
        |> List.partition_tf ~f:(Pos.overlaps result.res_span)
      in
      (* Make sure the primary error position is the latest position in the
         callable being analysed *)
      let (primary_pos, other_poss) =
        match List.sort ~compare:Pos.compare primary_poss |> List.rev with
        | [] -> (result.res_span, other_poss)
        | primary :: primary_poss ->
          (primary, List.unordered_append primary_poss other_poss)
      in

      let (source, sink) = (to_err source, to_err sink) in
      Errors.illegal_information_flow primary_pos other_poss source sink
    in

    let context_implicit_policy_leakage (pos, source, sink) =
      (* The latest program point contributing to the violation is the
         primary error *)
      let (primary, secondaries) =
        let poss =
          PosSet.elements pos |> List.sort ~compare:Pos.compare |> List.rev
        in
        match poss with
        | [] -> (result.res_span, [])
        | primary :: secondaries -> (primary, secondaries)
      in
      let (source, sink) = (to_err source, to_err sink) in
      Errors.context_implicit_policy_leakage primary secondaries source sink
    in

    if should_print ~user_mode:opts.opt_mode ~phase:Mcheck then begin
      List.iter ~f:illegal_information_flow simple_illegal_flows;
      List.iter ~f:context_implicit_policy_leakage implicit
    end
  in
  SMap.iter check_valid_flow simplified_results

let do_ opts files_info ctx =
  ( if should_print ~user_mode:opts.opt_mode ~phase:Mlattice then
    let lattice = opts.opt_security_lattice in
    Format.printf "@[Lattice:@. %a@]\n\n" Pp.security_lattice lattice );

  let handle_file path info errors =
    match info.FileInfo.file_mode with
    | Some FileInfo.Mstrict ->
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      let { Tast_provider.Compute_tast.tast; _ } =
        Tast_provider.compute_tast_unquarantined ~ctx ~entry
      in
      let check () = check opts tast in
      let (new_errors, _) = Errors.do_with_context path Errors.Typing check in
      errors @ Errors.get_error_list new_errors
    | _ -> errors
  in

  Relative_path.Map.fold files_info ~init:[] ~f:handle_file

let magic_builtins =
  [|
    ( "ifc_magic.hhi",
      {|<?hh // strict
class InferFlows
  implements
    HH\FunctionAttribute,
    HH\MethodAttribute {}
class Policied
  implements
    HH\InstancePropertyAttribute,
    HH\ClassAttribute,
    HH\ParameterAttribute,
    HH\FunctionAttribute {
  public function __construct(public string $purpose) { }
}
class Governed
 implements
   HH\FunctionAttribute,
   HH\MethodAttribute {
  public function __construct(public string $purpose = "") { }
}
class External
  implements
    HH\ParameterAttribute {}
|}
    );
  |]
