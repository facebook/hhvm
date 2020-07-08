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
module Solver = Ifc_solver
module Utils = Ifc_utils
module A = Aast
module T = Typing_defs
module L = Logic.Infix
module K = Typing_cont_key

exception FlowInference of string

let fail fmt = Format.kasprintf (fun s -> raise (FlowInference s)) fmt

(* A constraint accumulator that registers a subtyping
   requirement t1 <: t2 *)
let rec subtype t1 t2 acc =
  match (t1, t2) with
  | (Tprim p1, Tprim p2) -> L.(p1 < p2) acc
  | (Ttuple tl1, Ttuple tl2) ->
    (match List.zip tl1 tl2 with
    | Some zip ->
      List.fold zip ~init:acc ~f:(fun acc (t1, t2) -> subtype t1 t2 acc)
    | None -> fail "incompatible tuple types")
  | (Tclass cl1, Tclass cl2) when String.equal cl1.c_name cl2.c_name ->
    let policied_properties_zip =
      match
        List.zip
          (SMap.values cl1.c_property_map)
          (SMap.values cl2.c_property_map)
      with
      | Some zip -> zip
      | None -> fail "same class with differing policied properties"
    in
    let tparams_subtype acc =
      let tparam_subtype acc (pty1, variance1) (pty2, variance2) =
        assert (Ast_defs.equal_variance variance1 variance2);
        match variance1 with
        | Ast_defs.Invariant -> equivalent pty1 pty2 acc
        | Ast_defs.Covariant -> subtype pty1 pty2 acc
        | Ast_defs.Contravariant -> subtype pty2 pty1 acc
      in
      match
        List.fold2 ~init:acc ~f:tparam_subtype cl1.c_tparams cl2.c_tparams
      with
      | List.Or_unequal_lengths.Ok acc -> acc
      | _ -> fail "unequal number of type parameters during subtyping"
    in
    (* Invariant in property policies *)
    (* When forcing an unevaluated ptype thunk, only policied properties
     * will be populated. This invariant, and the ban on recursive class cycles
     * through policied properties (enforcement still to be done, see T68078692)
     * ensures termination here.
     *)
    List.fold policied_properties_zip ~init:acc ~f:(fun acc (t1, t2) ->
        equivalent (Lazy.force t1) (Lazy.force t2) acc)
    (* Use the declared variance for subtyping constraints of
     * type parameters *)
    |> tparams_subtype
    (* Invariant in lump policy *)
    |> L.(cl1.c_lump = cl2.c_lump)
    (* Covariant in class policy *)
    |> L.(cl1.c_self < cl2.c_self)
  | (Tunion tl, _) ->
    List.fold tl ~init:acc ~f:(fun acc t1 -> subtype t1 t2 acc)
  | (pty1, pty2) ->
    fail "unhandled subtyping query for %a <: %a" Pp.ptype pty1 Pp.ptype pty2

(* A constraint accumulator that registers that t1 = t2 *)
and equivalent t1 t2 acc = subtype t1 t2 (subtype t2 t1 acc)

(* Generates a fresh supertype of the argument type *)
let weaken ?(prefix = "weak") renv env ty =
  let rec freshen acc ty =
    let on_list mk tl =
      let (acc, tl') = List.map_env acc tl ~f:freshen in
      (acc, mk tl')
    in
    match ty with
    | Tprim p ->
      let p' = Env.new_policy_var renv.re_proto prefix in
      (L.(p < p') acc, Tprim p')
    | Ttuple tl -> on_list (fun l -> Ttuple l) tl
    | Tunion tl -> on_list (fun l -> Tunion l) tl
    | Tinter tl -> on_list (fun l -> Tinter l) tl
    | Tclass class_ ->
      let weaken_tparam acc ((pty, variance) as tparam) =
        match variance with
        | Ast_defs.Covariant ->
          let (acc, pty) = freshen acc pty in
          (acc, (pty, variance))
        | _ -> (acc, tparam)
      in
      let super_pol = Env.new_policy_var renv.re_proto prefix in
      let acc = L.(class_.c_self < super_pol) acc in
      let (acc, tparams) = List.map_env ~f:weaken_tparam acc class_.c_tparams in
      (acc, Tclass { class_ with c_self = super_pol; c_tparams = tparams })
  in
  let (acc, ty') = freshen env.e_acc ty in
  (Env.acc env (fun _ -> acc), ty')

(* A constraint accumulator registering that the type t depends on
   policies in the list pl (seen differently, the policies in pl
   flow into the type t) *)
let rec add_dependencies pl t acc =
  match t with
  | Tinter [] ->
    (* The policy p flows into a value of type mixed.
       Here we choose that mixed will be public; we
       could choose a different default policy or
       have mixed carry a policy (preferable). *)
    L.(pl <* [Pbot]) acc
  | Tprim pol
  (* For classes, we only add a dependency to the class policy and not to its
     properties *)
  | Tclass { c_self = pol; _ } ->
    L.(pl <* [pol]) acc
  | Tunion tl
  | Ttuple tl
  | Tinter tl ->
    let f acc t = add_dependencies pl t acc in
    List.fold tl ~init:acc ~f

let policy_join renv env ?(prefix = "join") p1 p2 =
  match Logic.policy_join p1 p2 with
  | Some p -> (env, p)
  | None ->
    (match (p1, p2) with
    | (Pfree_var (v1, s1), Pfree_var (v2, s2))
      when equal_policy_var v1 v2 && Scope.equal s1 s2 ->
      (env, p1)
    | _ ->
      let pv = Env.new_policy_var renv.re_proto prefix in
      let env = Env.acc env L.(p1 < pv && p2 < pv) in
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

(* If there is a lump policy variable in effect, return that otherwise
   generate a new policy variable. *)
let get_policy ?prefix lump_pol_opt proto_renv =
  match lump_pol_opt with
  | Some lump_pol -> lump_pol
  | None ->
    let prefix = Option.value prefix ~default:"v" in
    Env.new_policy_var proto_renv prefix

let rec class_ptype lump_pol_opt proto_renv tparams name =
  let { cd_policied_properties; cd_tparam_variance } =
    match SMap.find_opt name proto_renv.pre_decl.de_class with
    | Some class_sig -> class_sig
    | None -> fail "could not found a class policy signature for %s" name
  in
  let prop_ptype { pp_name; pp_type; pp_purpose } =
    (* Purpose of the property takes precedence over any lump policy. *)
    let lump_pol_opt =
      Option.merge
        (Option.map ~f:Ifc_security_lattice.parse_policy pp_purpose)
        lump_pol_opt
        ~f:(fun a _ -> a)
    in
    ( pp_name,
      lazy (ptype ~prefix:("." ^ pp_name) lump_pol_opt proto_renv pp_type) )
  in
  let lump_pol = get_policy lump_pol_opt proto_renv ~prefix:"lump" in
  let c_tparams =
    let prefix = "tp" in
    let pair_tparam tp var = (ptype ~prefix lump_pol_opt proto_renv tp, var) in
    match List.map2 ~f:pair_tparam tparams cd_tparam_variance with
    | List.Or_unequal_lengths.Ok zip -> zip
    | _ -> fail "unequal number of type parameters and variance spec"
  in
  Tclass
    {
      c_name = name;
      c_self = get_policy lump_pol_opt proto_renv ~prefix:name;
      c_lump = lump_pol;
      c_property_map =
        SMap.of_list (List.map ~f:prop_ptype cd_policied_properties);
      c_tparams;
    }

(* Turns a locl_ty into a type with policy annotations;
   the policy annotations are fresh policy variables *)
and ptype ?prefix lump_pol_opt proto_renv (t : T.locl_ty) =
  let ptype = ptype ?prefix lump_pol_opt proto_renv in
  match T.get_node t with
  | T.Tprim _ -> Tprim (get_policy lump_pol_opt proto_renv ?prefix)
  | T.Ttuple tyl -> Ttuple (List.map ~f:ptype tyl)
  | T.Tunion tyl -> Tunion (List.map ~f:ptype tyl)
  | T.Tintersection tyl -> Tinter (List.map ~f:ptype tyl)
  | T.Tclass ((_, name), _, tparams) ->
    class_ptype lump_pol_opt proto_renv tparams name
  | T.Tvar id ->
    (* Drops the environment `expand_var` returns. This is logically
     * correct, but threading the envrionment would lead to faster future
     * `Tvar` lookups.
     *)
    let (_, ty) =
      Typing_inference_env.expand_var
        proto_renv.pre_tenv.Tast.inference_env
        Typing_reason.Rnone
        id
    in
    ptype ty
  (* ---  types below are not yet unpported *)
  | T.Tdependent (_, _ty) -> fail "Tdependent"
  | T.Tdarray (_keyty, _valty) -> fail "Tdarray"
  | T.Tvarray _ty -> fail "Tvarray"
  | T.Tvarray_or_darray (_keyty, _valty) -> fail "Tvarray_or_darray"
  | T.Tany _sentinel -> fail "Tany"
  | T.Terr -> fail "Terr"
  | T.Tnonnull -> fail "Tnonnull"
  | T.Tdynamic -> fail "Tdynamic"
  | T.Toption _ty -> fail "Toption"
  | T.Tfun _fun_ty -> fail "Tfun"
  | T.Tshape (_sh_kind, _sh_type_map) -> fail "Tshape"
  | T.Tgeneric _name -> fail "Tgeneric"
  | T.Tnewtype (_name, _ty_list, _as_bound) -> fail "Tnewtype"
  | T.Tobject -> fail "Tobject"
  | T.Tpu (_locl_ty, _sid) -> fail "Tpu"
  | T.Tpu_type_access (_sid1, _sid2) -> fail "Tpu_type_access"

(* Uses a Hack-inferred type to update the flow type of a local
   variable *)
let refresh_lvar_type renv env lid (ety : T.locl_ty) =
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
    let new_pty = ptype None renv.re_proto ety ~prefix in
    let env = Env.acc env (subtype pty new_pty) in
    let env = Env.set_local_type env lid new_pty in
    (env, new_pty)

let add_params renv =
  let add_param env p =
    let prefix = p.A.param_name in
    let pty = ptype None renv.re_proto (fst p.A.param_type_hint) ~prefix in
    let lid = Local_id.make_unscoped p.A.param_name in
    let env = Env.set_local_type env lid pty in
    (env, pty)
  in
  List.map_env ~f:add_param

let binop renv env ty1 ty2 =
  match (ty1, ty2) with
  | (Tprim p1, Tprim p2) ->
    let (env, pj) = policy_join renv env ~prefix:"bop" p1 p2 in
    (env, Tprim pj)
  | _ -> fail "unexpected Binop types"

let receiver_of_obj_get obj_ptype property =
  match obj_ptype with
  | Tclass class_ -> class_
  | _ -> fail "couldn't find a class for the property '%s'" property

let property_ptype proto_renv obj_ptype property property_ty =
  let class_ = receiver_of_obj_get obj_ptype property in
  match SMap.find_opt property class_.c_property_map with
  | Some ptype -> Lazy.force ptype
  | None ->
    ptype ~prefix:("." ^ property) (Some class_.c_lump) proto_renv property_ty

let call renv env callable_name that_pty_opt args_pty ret_ty =
  let ret_pty =
    let prefix = callable_name ^ "_ret" in
    ptype ~prefix None renv.re_proto ret_ty
  in
  let env =
    match SMap.find_opt callable_name renv.re_proto.pre_decl.de_fun with
    (* TODO(T68007489): Temporarily infer everything for every function call.
     * switch back to using InferFlows annotation when scaling is an issue.
     *)
    | Some _ ->
      let (env, pc_joined) =
        let join (env, pc) pc' = policy_join renv env ~prefix:"pcjoin" pc pc' in
        List.fold ~f:join ~init:(env, Pbot) (Env.get_gpc_policy env K.Next)
      in
      let fp =
        {
          fp_name = callable_name;
          fp_this = that_pty_opt;
          fp_pc = pc_joined;
          fp_args = args_pty;
          fp_ret = ret_pty;
        }
      in
      let env = Env.acc env (fun acc -> Chole fp :: acc) in
      let env = Env.add_dep env callable_name in
      env
    | None -> fail "unknown function '%s'" callable_name
  in
  (env, ret_pty)

let vec_element_pty vec_pty =
  let class_ =
    match vec_pty with
    | Tclass cls when String.equal cls.c_name "\\HH\\vec" -> cls
    | _ -> fail "expected a vector"
  in
  match class_.c_tparams with
  | [(element_pty, _)] -> element_pty
  | _ -> fail "expected one type parameter from a vector object"

(* Finds what we flow into in an assignment.
 * The type LHS has in the input is the type after the assignment takes place. *)
let rec flux_target renv env ((_, lhs_ty), lhs_exp) =
  match lhs_exp with
  | A.Lvar (_, lid) ->
    let prefix = Local_id.to_string lid in
    let local_pty = ptype None renv.re_proto lhs_ty ~prefix in
    let env = Env.set_local_type env lid local_pty in
    (env, local_pty, Env.get_lpc_policy env K.Next)
  | A.Obj_get (obj, (_, A.Id (_, property)), _) ->
    let (env, obj_pty) = expr renv env obj in
    let obj_pol = (receiver_of_obj_get obj_pty property).c_self in
    let prop_pty = property_ptype renv.re_proto obj_pty property lhs_ty in
    let env = Env.acc env (add_dependencies [obj_pol] prop_pty) in
    (env, prop_pty, Env.get_gpc_policy env K.Next)
  | A.Array_get (vec, ix_opt) ->
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
      | Some ix -> fst @@ expr renv env ix
      | None -> env
    in
    let (env, old_vec_pty) = expr renv env vec in
    let (env, vec_pty, pc) = flux_target renv env vec in
    let env = Env.acc env (subtype old_vec_pty vec_pty) in
    let element_pty = vec_element_pty vec_pty in
    (* Even though the runtime updates the entire vector, we treat
     * the mutation as if it were happening on a single element.
     * That is sound because, after the mutation, changes can only
     * be observed via the updated element.
     *)
    (env, element_pty, pc)
  | _ -> fail "unhandled flux target (lvalue)"

and assign renv env op lhs_exp rhs_exp =
  (* Handle the incorporation of LHS in RHS if assignment uses and operation,
   * e.g., $a += $b. *)
  let (env, rhs_pty) =
    if Option.is_none op then
      expr renv env rhs_exp
    else
      let (env, lhs_pty) = expr renv env lhs_exp in
      let (env, rhs_pty) = expr renv env rhs_exp in
      binop renv env lhs_pty rhs_pty
  in
  let (env, lhs_pty, pc) = flux_target renv env lhs_exp in
  let env = Env.acc env (subtype rhs_pty lhs_pty) in
  let env = Env.acc env (add_dependencies pc lhs_pty) in
  (env, lhs_pty)

(* Generate flow constraints for an expression *)
and expr renv env (((_epos, ety), e) : Tast.expr) =
  let expr = expr renv in
  match e with
  | A.True
  | A.False
  | A.Int _
  | A.Float _
  | A.String _ ->
    (* literals are public *)
    (env, Tprim Pbot)
  | A.Binop (Ast_defs.Eq op, e1, e2) -> assign renv env op e1 e2
  | A.Binop (_, e1, e2) ->
    let (env, ty1) = expr env e1 in
    let (env, ty2) = expr env e2 in
    binop renv env ty1 ty2
  | A.Lvar (_pos, lid) -> refresh_lvar_type renv env lid ety
  | A.Obj_get (obj, (_, A.Id (_, property)), _) ->
    let (env, obj_ptype) = expr env obj in
    let prop_ptype = property_ptype renv.re_proto obj_ptype property ety in
    let (env, super_ptype) =
      weaken ~prefix:("." ^ property) renv env prop_ptype
    in
    let obj_class = receiver_of_obj_get obj_ptype property in
    let env = Env.acc env (add_dependencies [obj_class.c_self] super_ptype) in
    (env, super_ptype)
  | A.This ->
    (match renv.re_this with
    | Some ptype -> (env, ptype)
    | None -> fail "encountered $this outside of a class context")
  | A.BracedExpr e -> expr env e
  | A.Call (_call_type, (_, A.Id (_, name)), _type_args, args, _extra_args) ->
    let (env, args_pty) = List.map_env ~f:expr env args in
    let callable_name = Decl.make_callable_name None name in
    call renv env callable_name None args_pty ety
  | A.Call (_, (_, A.Obj_get (obj, (_, A.Id (_, meth_name)), _)), _, args, _) ->
    let (env, args_pty) = List.map_env ~f:expr env args in
    let (env, obj_pty) = expr env obj in
    (match obj_pty with
    | Tclass class_ ->
      let callable_name =
        Decl.make_callable_name (Some class_.c_name) meth_name
      in
      call renv env callable_name (Some obj_pty) args_pty ety
    | _ -> fail "unhandled method call on %a" Pp.ptype obj_pty)
  | A.ValCollection (A.Vec, _, exprs) ->
    (* We require each collection element to be a subtype of the vector
     * element parameter. *)
    let vec_pty = ptype ~prefix:"vec" None renv.re_proto ety in
    let element_pty = vec_element_pty vec_pty in
    let mk_element_subtype env exp =
      let (env, pty) = expr env exp in
      Env.acc env (subtype pty element_pty)
    in
    let env = List.fold ~f:mk_element_subtype ~init:env exprs in
    (env, vec_pty)
  | A.Array_get (exp, ix_opt) ->
    (* Return the type parameter corresponding to the vector element type. *)
    let env =
      (* Evaluate the index in case it has side-effects. *)
      match ix_opt with
      | Some ix -> fst @@ expr env ix
      | None -> fail "cannot have an empty index when reading"
    in
    let (env, vec_pty) = expr env exp in
    let element_pty = vec_element_pty vec_pty in
    (env, element_pty)
  (* --- expressions below are not yet supported *)
  | A.Array _
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
  | A.Call (_, _, _, _, _)
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
  | A.New (_, _, _, _, _)
  | A.Record (_, _)
  | A.Efun (_, _)
  | A.Lfun (_, _)
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

let rec stmt renv env ((_pos, s) : Tast.stmt) =
  match s with
  | A.Expr e ->
    let (env, _ty) = expr renv env e in
    env
  | A.If (e, b1, b2) ->
    let (env, ety) = expr renv env e in
    (* Stash the PC's so they can be restored after the if *)
    let gpc = Env.get_gpc_policy env K.Next in
    let lpc = Env.get_lpc_policy env K.Next in
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
    Env.set_pcs env K.Next gpc lpc
  | A.Return e ->
    let union env t1 t2 = (env, mk_union t1 t2) in
    let env = Env.merge_conts_into ~union env [K.Next] K.Exit in
    begin
      match e with
      | None -> env
      | Some e ->
        let (env, te) = expr renv env e in
        (* to account for enclosing conditionals, make the return
          type depend on the local pc *)
        let lpc = Env.get_lpc_policy env K.Next in
        Env.acc
          env
          L.(add_dependencies lpc renv.re_ret && subtype te renv.re_ret)
    end
  | _ -> env

and block renv env (blk : Tast.block) =
  let seq env s =
    let env = Env.merge_pcs_into env [K.Exit] K.Next in
    stmt renv env s
  in
  List.fold_left ~f:seq ~init:env blk

let callable opts decl_env class_name_opt name saved_env params body lrty =
  try
    (* Setup the read-only environment *)
    let scope = Scope.alloc () in
    let proto_renv = Env.new_proto_renv saved_env scope decl_env in

    let global_pc = Env.new_policy_var proto_renv "pc" in
    (* Here, we ignore the type parameters of this because at the moment we
     * lack Tgeneric policy type. This will be fixed (T68414656) in the future.
     *)
    let this_ty = Option.map class_name_opt (class_ptype None proto_renv []) in
    let ret_ty = ptype ~prefix:"ret" None proto_renv lrty in
    let renv = Env.new_renv proto_renv this_ty ret_ty in

    (* Initialise the mutable environment *)
    let env = Env.new_env global_pc in
    let (env, param_tys) = add_params renv env params in

    (* Run the analysis *)
    let beg_env = env in
    let env = block renv env body.A.fb_ast in
    let end_env = env in

    (* Display the analysis results *)
    if opts.verbosity >= 2 then begin
      Format.printf "Analyzing %s:@." name;
      Format.printf "%a@." Pp.renv renv;
      Format.printf "* Params:@,  %a@." Pp.locals beg_env;
      Format.printf "* Final environment:@,  %a@." Pp.env end_env;
      Format.printf "@."
    end;

    (* Return the results *)
    let res =
      let proto =
        {
          fp_name = Decl.make_callable_name class_name_opt name;
          fp_pc = global_pc;
          fp_this = this_ty;
          fp_args = param_tys;
          fp_ret = ret_ty;
        }
      in
      {
        res_proto = proto;
        res_scope = scope;
        res_constraint = Logic.conjoin env.e_acc;
        res_deps = env.e_deps;
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
          f_ret = (lrty, _);
          _;
        } ->
      Option.map
        ~f:(fun x -> [x])
        (callable opts decl_env None name saved_env params body lrty)
    | A.Class { A.c_name = (_, class_name); c_methods = methods; _ } ->
      let handle_method
          {
            A.m_name = (_, name);
            m_annotation = saved_env;
            m_params = params;
            m_body = body;
            m_ret = (lrty, _);
            _;
          } =
        callable opts decl_env (Some class_name) name saved_env params body lrty
      in
      Some (List.filter_map ~f:handle_method methods)
    | _ -> None
  in
  (fun tast -> List.concat (List.filter_map ~f:def tast))

let do_ opts files_info ctx =
  Relative_path.Map.iter files_info ~f:(fun path i ->
      (* skip decls and partial *)
      match i.FileInfo.file_mode with
      | Some FileInfo.Mstrict ->
        let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        let { Tast_provider.Compute_tast.tast; _ } =
          Tast_provider.compute_tast_unquarantined ~ctx ~entry
        in
        let decl_env = Decl.collect_sigs tast in
        if opts.verbosity >= 3 then Format.printf "%a@." Pp.decl_env decl_env;

        let results = walk_tast opts decl_env tast in

        let results =
          try Solver.global_exn ~subtype results with
          | Solver.Error Solver.RecursiveCycle ->
            fail "solver error: cyclic call graph"
          | Solver.Error (Solver.MissingResults callable) ->
            fail "solver error: missing results for callable '%s'" callable
          | Solver.Error (Solver.InvalidCall (reason, caller, callee)) ->
            fail
              "solver error: invalid call to '%s' in '%s' (%s)"
              callee
              caller
              reason
        in

        let simplify result =
          let pred = const true in
          ( result,
            Logic.simplify
            @@ Logic.quantify ~pred ~quant:Qexists result.res_constraint )
        in
        let simplified_results = SMap.map simplify results in

        let log_solver name (result, simplified) =
          Format.printf "@[<v>";
          Format.printf "Flow constraints for %s:@.  @[<v>" name;
          Format.printf "@,@[<hov>Simplified:@ @[<hov>%a@]@]" Pp.prop simplified;
          let raw = result.res_constraint in
          Format.printf "@,@[<hov>Raw:@ @[<hov>%a@]@]" Pp.prop raw;
          Format.printf "@]";
          Format.printf "@]\n\n"
        in
        if opts.verbosity >= 1 then SMap.iter log_solver simplified_results;

        let lattice =
          try Ifc_security_lattice.mk_exn opts.security_lattice
          with Ifc_security_lattice.Invalid_security_lattice ->
            fail
              "lattice parsing error: lattice specification should be `;` basic flux constraints, e.g., `A < B`"
        in
        let log_checking name (_, simple) =
          let violations =
            try Ifc_security_lattice.check_exn lattice simple
            with Ifc_security_lattice.Checking_error ->
              fail
                "lattice checking error: something went wrong while checking %a against %s"
                Pp.prop
                simple
                opts.security_lattice
          in

          if not @@ List.is_empty violations then begin
            Format.printf "There are privacy policy errors in %s:@.  @[<v>" name;
            List.iter ~f:(Format.printf "%a@," Pp.violation) violations;
            Format.printf "@]\n"
          end
        in
        SMap.iter log_checking simplified_results
      | _ -> ());
  ()

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
  public function __construct(public string $purpose = "") { }
}
|}
    );
  |]
