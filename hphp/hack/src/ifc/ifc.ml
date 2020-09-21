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
module Try = Typing_try

exception FlowInference of string

let should_print ~user_mode ~phase =
  equal_mode user_mode Mdebug || equal_mode user_mode phase

let fail fmt = Format.kasprintf (fun s -> raise (FlowInference s)) fmt

exception SubtypeFailure of string * ptype * ptype

(* A constraint accumulator that registers a subtyping
   requirement t1 <: t2 *)
let rec subtype ~pos t1 t2 acc =
  let subtype = subtype ~pos in
  let err msg = raise (SubtypeFailure (msg, t1, t2)) in
  let rec first_ok ~f msg = function
    | [] -> err msg
    | x :: l ->
      begin
        try f x with SubtypeFailure (msg, _, _) -> first_ok ~f msg l
      end
  in
  match (t1, t2) with
  | (Tprim p1, Tprim p2)
  | (Tgeneric p1, Tgeneric p2) ->
    L.(p1 < p2) ~pos acc
  | (Ttuple tl1, Ttuple tl2) ->
    (match List.zip tl1 tl2 with
    | Some zip ->
      List.fold zip ~init:acc ~f:(fun acc (t1, t2) -> subtype t1 t2 acc)
    | None -> err "incompatible tuple types")
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
      | None -> err "functions have different number of arguments"
    in
    (* Contravariant in argument types *)
    List.fold ~f:(fun acc (t1, t2) -> subtype t2 t1 acc) ~init:acc zipped_args
    (* Contravariant in PC *)
    |> L.(f2.f_pc < f1.f_pc) ~pos
    (* Covariant in its own identity *)
    |> L.(f1.f_self < f2.f_self) ~pos
    (* Covariant in return type and exceptions *)
    |> subtype f1.f_ret f2.f_ret
    |> subtype f1.f_exn f2.f_exn
  | (Tunion tl, _) ->
    List.fold tl ~init:acc ~f:(fun acc t1 -> subtype t1 t2 acc)
  | (_, Tinter tl) ->
    List.fold tl ~init:acc ~f:(fun acc t2 -> subtype t1 t2 acc)
  | (_, Tunion tl) ->
    (* It is not complete to try all the RHS elements in sequence
       but that's how typing_subtype.ml works today *)
    first_ok "nothing" tl ~f:(fun t2 -> subtype t1 t2 acc)
  | (Tinter tl, _) ->
    (* Same remark as for (_ <: Tunion _) *)
    first_ok "mixed" tl ~f:(fun t1 -> subtype t1 t2 acc)
  | _ -> err "unhandled subtyping query"

(* A constraint accumulator that registers that t1 = t2 *)
let equivalent ~pos t1 t2 acc =
  let subtype = subtype ~pos in
  subtype t1 t2 (subtype t2 t1 acc)

(* Overwrite subtype and equivalent catching the SubtypeFailure
   exception *)
let (subtype, equivalent) =
  let wrap f ~pos t1 t2 acc =
    try f ~pos t1 t2 acc
    with SubtypeFailure (msg, tsub, tsup) ->
      fail "subtype: %s (%a <: %a)" msg Pp.ptype tsub Pp.ptype tsup
  in
  (wrap subtype, wrap equivalent)

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
    let simple_freshen env f p =
      let (env, p) = freshen_pol_cov env p in
      (env, f p)
    in
    let on_list env mk tl =
      let (env, tl') = List.map_env env tl ~f:freshen_cov in
      (env, mk tl')
    in
    match ty with
    | Tprim p -> simple_freshen env (fun p -> Tprim p) p
    | Tgeneric p -> simple_freshen env (fun p -> Tgeneric p) p
    | Ttuple tl -> on_list env (fun l -> Ttuple l) tl
    | Tunion tl -> on_list env (fun l -> Tunion l) tl
    | Tinter tl -> on_list env (fun l -> Tinter l) tl
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

(* Returns the set of policies, to be understood as a join,
   that governs an object with the argument type *)
let rec object_policy = function
  | Tprim pol
  | Tgeneric pol
  | Tclass { c_self = pol; _ }
  | Tfun { f_self = pol; _ } ->
    PSet.singleton pol
  | Tinter tl
  | Tunion tl
  | Ttuple tl ->
    let f set t = PSet.union (object_policy t) set in
    List.fold tl ~init:PSet.empty ~f

let add_dependencies ~pos pl t acc =
  L.(pl <* PSet.elements (object_policy t)) ~pos acc

let policy_join ?(prefix = "join") renv p1 p2 =
  let id ~pos:_ acc = acc in
  match Logic.policy_join p1 p2 with
  | Some p -> (id, p)
  | None when equal_policy p1 p2 -> (id, p1)
  | _ ->
    let pv = Env.new_policy_var renv prefix in
    (L.(p1 < pv && p2 < pv), pv)

let policy_join_env ?prefix ~pos renv env p1 p2 =
  let (facc, p) = policy_join ?prefix renv p1 p2 in
  (Env.acc env (facc ~pos), p)

(* This function only needs to be sound: true is returned
   only if the two types are equal for sure, but false
   could be returned when the two types are same *)
let sound_equal_ptype (pty1 : ptype) pty2 = phys_equal pty1 pty2

let union env t1 t2 =
  if sound_equal_ptype t1 t2 then
    (env, t1)
  else
    (env, Tunion [t1; t2])

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
  let pty =
    match Env.get_local_type env lid with
    | None ->
      Errors.unknown_information_flow
        pos
        ("variable `" ^ Local_id.get_name lid ^ "`");
      Lift.ty renv ety
    | Some pty -> pty
  in
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
    let (env, pj) = policy_join_env ~pos renv env ~prefix:"bop" p1 p2 in
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
  let env = Env.merge_conts_into ~union env [K.Next] K.Catch in
  let lpc = Env.get_lpc env K.Next in
  Env.acc
    env
    (L.(
       subtype exn_ty renv.re_exn
       && add_dependencies (PSet.elements lpc) renv.re_exn)
       ~pos)

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
      policy_join_env ~pos renv env ~prefix:"pcjoin" pc pc'
    in
    PSet.fold join (Env.get_pc renv env K.Next) (env, callee)
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
  let env = Env.push_pcs env K.Next (PSet.singleton callee_exn_policy) in
  let env = throw ~pos renv env callee_exn in
  (env, ret_pty)

(* Lifts the element locl type found in vec_ty using the lump
   policy of the vec_pty policied type (must be a \Vec class) *)
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

(* Deals with a true assignment to either a local variable
   or an object property *)
let asn ~expr ~pos renv env ((_, lhs_ty), lhs_exp) rhs_pty =
  match lhs_exp with
  | A.Lvar (_, lid) ->
    let prefix = Local_id.to_string lid in
    let lhs_pty = Lift.ty ~prefix renv lhs_ty in
    (* set asn to true to mark the local as assigned in the
       current code branch *)
    let env = Env.set_local_type env lid lhs_pty in
    let deps = PSet.elements (Env.get_lpc env K.Next) in
    let env = Env.acc env (add_dependencies ~pos deps lhs_pty) in
    let env = Env.acc env (subtype ~pos rhs_pty lhs_pty) in
    env
  | A.Obj_get (obj, (_, A.Id (_, property)), _) ->
    let (env, obj_pty) = expr env obj in
    let obj_pol = (receiver_of_obj_get obj_pty property).c_self in
    let lhs_pty = property_ptype renv obj_pty property lhs_ty in
    let deps = obj_pol :: PSet.elements (Env.get_pc renv env K.Next) in
    let env = Env.acc env (add_dependencies ~pos deps lhs_pty) in
    let env = Env.acc env (subtype ~pos rhs_pty lhs_pty) in
    env
  | _ ->
    Errors.unknown_information_flow pos "lvalue";
    env

(* A wrapper for asn that deals with Hack arrays' syntactic
   sugar; it is important to get it right to account for the
   CoW semantics of Hack arrays:

     $x->p[4][] = "hi";

   does not mutate an array cell, but the property p of
   the object $x instead.  *)
let rec asn_top ~expr ~pos renv env lhs rhs_pty =
  match snd lhs with
  | A.Array_get ((((_, vec_ty), _) as vec), ix_opt) ->
    (* TODO(T68269878): track flows due to length changes *)
    let (env, _ix_ty_opt) =
      match ix_opt with
      | Some ix ->
        let (env, ty) = expr env ix in
        (env, Some ty)
      | None -> (env, None)
    in
    (* we now compute the type of the new array resulting
       from the insertion/update *)
    let (env, vec_pty) =
      let (env, old_vec_pty) = expr env vec in
      let (env, vec_pty) =
        adjust_ptype ~pos ~adjustment:Aweaken renv env old_vec_pty
      in
      (* the rhs flows into the element type *)
      let elem_pty = vec_element_pty renv vec_ty vec_pty in
      let env = Env.acc env (subtype ~pos rhs_pty elem_pty) in
      (env, vec_pty)
    in
    (* assign the vector itself *)
    asn_top ~expr ~pos renv env vec vec_pty
  | _ -> asn ~expr ~pos renv env lhs rhs_pty

let rec assign ~pos renv env op lhs_exp rhs_exp =
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
  let env = asn_top ~expr ~pos renv env lhs_exp rhs_pty in
  (env, rhs_pty)

(* Generate flow constraints for an expression *)
and expr ~pos renv env (((_epos, ety), e) : Tast.expr) =
  let expr = expr ~pos renv in
  match e with
  | A.Null
  | A.True
  | A.False
  | A.Int _
  | A.Float _
  | A.String _ ->
    (* literals are public *)
    (env, Tprim (Env.new_policy_var renv "lit"))
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
  | A.ExpressionTree (_, e, _)
  | A.BracedExpr e ->
    expr env e
  (* TODO(T68414656): Support calls with type arguments *)
  | A.Call (e, _type_args, args, _extra_args) ->
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
            Errors.unknown_information_flow pos "late static binding";
            env
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
        Errors.unknown_information_flow pos "late static binding";
        env
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
    let env = Env.set_lpc env K.Next PSet.empty in
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
    Errors.unknown_information_flow pos "expression";
    (env, Lift.ty renv ety)

and stmt renv env ((pos, s) : Tast.stmt) =
  let expr = expr renv in
  match s with
  | A.Expr e ->
    let (env, _ty) = expr ~pos env e in
    env
  | A.If ((((pos, _), _) as e), b1, b2) ->
    let (env, ety) = expr ~pos env e in
    (* stash the PC so it can be restored after the if *)
    let pc = Env.get_lpc env K.Next in
    (* use object_policy to account for both booleans
       and null checks *)
    let env = Env.push_pcs env K.Next (object_policy ety) in
    let cenv = Env.get_cenv env in
    let env = block renv (Env.set_cenv env cenv) b1 in
    let cenv1 = Env.get_cenv env in
    let env = block renv (Env.set_cenv env cenv) b2 in
    let cenv2 = Env.get_cenv env in
    let env = Env.merge_and_set_cenv ~union env cenv1 cenv2 in
    (* Restore the program counter from before the IF *)
    Env.set_lpc env K.Next pc
  | A.Return e ->
    let env = Env.merge_conts_into ~union env [K.Next] K.Exit in
    begin
      match e with
      | None -> env
      | Some e ->
        let (env, te) = expr ~pos env e in
        (* to account for enclosing conditionals, make the return
          type depend on the local pc *)
        let lpc = Env.get_lpc env K.Next in
        Env.acc
          env
          (L.(
             add_dependencies (PSet.elements lpc) renv.re_ret
             && subtype te renv.re_ret)
             ~pos)
    end
  | A.Throw e ->
    let (env, exn_ty) = expr ~pos env e in
    throw ~pos renv env exn_ty
  | A.Try (try_blk, cs, finally) ->
    (* List of continuations that get masked by finally *)
    let masked_conts = [K.Break; K.Continue; K.Exit; K.Catch; K.Finally] in
    Env.with_fresh_conts ~union env masked_conts @@ fun env ->
    let base_pc = Env.get_lpc env K.Next in

    (* Create a fresh exception for the try block since none of the outer
     * exceptions are catchable inside the block
     *)
    let fresh_exn = Lift.class_ty renv Decl.exception_id in
    let try_renv = { renv with re_exn = fresh_exn } in
    let env = block try_renv env try_blk in
    let env = Env.move_conts_into ~union env [K.Next] K.Finally in

    (* The catch block should begin with the catch continuation replacing Next *)
    let catch_cenv =
      Env.move_conts_into ~union env [K.Catch] K.Next |> Env.get_cenv
    in

    (* In case there is no catch all, then the exceptions still linger *)
    let has_catch_all =
      let f ((_, exn), _, _) = String.equal exn Decl.exception_id in
      List.exists ~f cs
    in
    let env =
      if has_catch_all then
        Env.drop_conts env [K.Catch]
      else
        Env.acc env @@ subtype ~pos fresh_exn renv.re_exn
    in
    let finally_base = Env.get_cenv env in

    let catch env (_, (_, exn_var), blk) =
      let env = Env.set_cenv env catch_cenv in
      let env = Env.set_local_type env exn_var fresh_exn in
      let env = block renv env blk in
      let env = Env.move_conts_into ~union env [K.Next] K.Finally in
      (env, Env.get_cenv env)
    in

    (* Finally runs with the merged conts from try and all the catch blocks, but
     it gets the base pc because it runs regardless of whether an exception was
     thrown *)
    let (env, cenvs) = List.map_env ~f:catch env cs in
    let (env, merged_cenv) =
      let f (env, cenv1) cenv2 = Env.merge_cenvs ~union env cenv1 cenv2 in
      List.fold ~f ~init:(env, finally_base) cenvs
    in
    let env = Env.set_cenv env merged_cenv in

    (* Analyze finally against each of the continuations in order to get more
       precise flow information. *)
    let (env, cenvs) =
      let f env key lenv =
        let env = Env.set_cenv env @@ KMap.singleton K.Next lenv in
        let pc = Env.get_lpc env K.Next in
        let env = Env.set_lpc env K.Next base_pc in
        let env = block renv env finally in
        let env =
          match key with
          | K.Finally -> env
          | _ -> Env.set_lpc env K.Next pc
        in
        (env, Env.get_cenv env)
      in
      KMap.map_env f env @@ Env.get_cenv env
    in
    let (env, cenv) =
      let union = Utils.mk_combine true (Env.merge_lenv ~union) in
      Try.finally_merge union env cenvs masked_conts
    in
    let env = Env.set_cenv env cenv in
    Env.move_conts_into ~union env [K.Finally] K.Next
  | A.Noop -> env
  | _ ->
    Errors.unknown_information_flow pos "statement";
    env

and block renv env (blk : Tast.block) =
  let merge_pcs env = Env.merge_pcs_into env [K.Exit; K.Catch] K.Next in
  let seq env s = stmt renv env s |> merge_pcs in
  let env = merge_pcs env in
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
class CanCall
  implements
    HH\ParameterAttribute {}
|}
    );
  |]
