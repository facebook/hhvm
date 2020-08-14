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
module Opts = Ifc_options
module Pp = Ifc_pretty
module Lattice = Ifc_security_lattice
module Solver = Ifc_solver
module Utils = Ifc_utils
module A = Aast
module T = Typing_defs
module L = Logic.Infix
module K = Typing_cont_key
module TClass = Decl_provider.Class
module TReason = Typing_reason
module TPhase = Typing_phase
module TEnv = Typing_env
module TUtils = Typing_utils

exception FlowInference of string

let should_print ~user_mode ~phase =
  equal_mode user_mode Mdebug || equal_mode user_mode phase

let fail fmt = Format.kasprintf (fun s -> raise (FlowInference s)) fmt

let empty_tenv meta = TEnv.empty meta.m_ctx meta.m_path None

let find_class_decl meta class_name =
  match Decl_provider.get_class meta.m_ctx class_name with
  | Some class_decl -> class_decl
  | None -> fail "couldn't find the decl for %s" class_name

let find_variances meta class_name =
  let class_decl = find_class_decl meta class_name in
  let tparams = TClass.tparams class_decl in
  List.map ~f:(fun tparam -> tparam.T.tp_variance) tparams

let find_ancestor_tparams proto_renv class_name (targs : T.locl_ty list) =
  let class_decl = find_class_decl proto_renv.pre_meta class_name in
  let tparam_names =
    let tp_name tparam = snd tparam.T.tp_name in
    List.map ~f:tp_name @@ TClass.tparams class_decl
  in
  let substitution =
    match List.zip tparam_names targs with
    | Some zipped -> SMap.of_list zipped
    | None ->
      fail "%s has uneven number of type parameters and arguments" class_name
  in
  let extract_tparams (ancestor_name, ancestor) =
    let tenv = empty_tenv proto_renv.pre_meta in
    (* Type localisation configuration. We use it for instantiating type parameters. *)
    let ety_env =
      let this_ty = T.mk (TReason.none, TUtils.this_of @@ TEnv.get_self tenv) in
      {
        T.type_expansions = [];
        substs = substitution;
        quiet = false;
        on_error =
          (fun ?code:_ _ ->
            fail "something went wrong during generic substitution");
        this_ty;
        from_class = None;
      }
    in
    let (_, ty) = TPhase.localize ~ety_env tenv ancestor in
    match T.get_node ty with
    | T.Tclass (_, _, targs) ->
      if List.is_empty targs then
        None
      else
        Some (ancestor_name, targs)
    | _ ->
      fail
        "Tried to extract type parameters of %s, but it is not a class"
        ancestor_name
  in
  TClass.all_ancestors class_decl
  |> List.filter_map ~f:extract_tparams
  |> SMap.of_list

(* A constraint accumulator that registers a subtyping
   requirement t1 <: t2 *)
let rec subtype meta t1 t2 acc =
  let subtype = subtype meta in
  let equivalent = equivalent meta in
  match (t1, t2) with
  | (Tprim p1, Tprim p2)
  | (Tgeneric p1, Tgeneric p2) ->
    L.(p1 < p2) acc
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
      let tparam_subtype acc pty1 pty2 variance =
        match variance with
        | Ast_defs.Invariant -> equivalent pty1 pty2 acc
        | Ast_defs.Covariant -> subtype pty1 pty2 acc
        | Ast_defs.Contravariant -> subtype pty2 pty1 acc
      in
      let tparams_subtype acc class_name tparams_opt1 tparams_opt2 =
        let variances = find_variances meta class_name in
        match (tparams_opt1, tparams_opt2) with
        | (Some tparams1, Some tparams2) ->
          let init = acc in
          let f = tparam_subtype in
          begin
            match Utils.fold3 ~init ~f tparams1 tparams2 variances with
            | List.Or_unequal_lengths.Ok acc -> (acc, Some ())
            | _ -> fail "unequal number of type parameters during subtyping"
          end
        | _ -> (acc, None)
      in
      let combine = tparams_subtype in
      fst @@ SMap.merge_env acc ~combine cl1.c_tparams cl2.c_tparams
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
  | (Tfun f1, Tfun f2) ->
    let zipped_args =
      match List.zip f1.f_args f2.f_args with
      | Some zip -> zip
      | None -> failwith "functions have different number of arguments"
    in
    (* Contravariant in argument types *)
    List.fold ~f:(fun acc (t1, t2) -> subtype t2 t1 acc) ~init:acc zipped_args
    (* Contravariant in PC *)
    |> L.(f2.f_pc < f1.f_pc)
    (* Covariant in its own identity *)
    |> L.(f1.f_self < f2.f_self)
    (* Covariant in return type and exceptions *)
    |> L.(subtype f1.f_ret f2.f_ret && subtype f1.f_exn f2.f_exn)
  | (Tunion tl, _) ->
    List.fold tl ~init:acc ~f:(fun acc t1 -> subtype t1 t2 acc)
  | (pty1, pty2) ->
    fail "unhandled subtyping query for %a <: %a" Pp.ptype pty1 Pp.ptype pty2

(* A constraint accumulator that registers that t1 = t2 *)
and equivalent meta t1 t2 acc = subtype meta t1 t2 (subtype meta t2 t1 acc)

(* Generates a fresh supertype of the argument type *)
let adjust ?(prefix = "weak") ~adjustment renv env ty =
  let flip = function
    | Astrengthen -> Aweaken
    | Aweaken -> Astrengthen
  in
  let freshen_policy adjustment acc policy =
    let new_policy = Env.new_policy_var renv.re_proto prefix in
    let prop =
      match adjustment with
      | Astrengthen -> L.(new_policy < policy)
      | Aweaken -> L.(policy < new_policy)
    in
    (prop acc, new_policy)
  in
  let rec freshen adjustment acc ty =
    let freshen_cov = freshen adjustment in
    let freshen_con = freshen @@ flip adjustment in
    let freshen_pol_cov = freshen_policy adjustment in
    let freshen_pol_con = freshen_policy @@ flip adjustment in
    let simple_freshen f acc policy =
      let (acc, p) = freshen_pol_cov acc policy in
      (acc, f p)
    in
    let on_list mk tl =
      let (acc, tl') = List.map_env acc tl ~f:freshen_cov in
      (acc, mk tl')
    in
    match ty with
    | Tprim policy -> simple_freshen (fun p -> Tprim p) acc policy
    | Tgeneric policy -> simple_freshen (fun p -> Tgeneric p) acc policy
    | Ttuple tl -> on_list (fun l -> Ttuple l) tl
    | Tunion tl -> on_list (fun l -> Tunion l) tl
    | Tinter tl -> on_list (fun l -> Tinter l) tl
    | Tclass class_ ->
      let adjust_tparam acc variance pty =
        match variance with
        | Ast_defs.Covariant -> freshen_cov acc pty
        | Ast_defs.Invariant -> (acc, pty)
        | Ast_defs.Contravariant -> freshen_con acc pty
      in
      let (acc, super_pol) = freshen_pol_cov acc class_.c_self in
      let (acc, tparams) =
        let adjust_tparams acc class_name =
          let variances = find_variances renv.re_proto.pre_meta class_name in
          List.map2_env ~f:adjust_tparam acc variances
        in
        SMap.map_env adjust_tparams acc class_.c_tparams
      in
      (acc, Tclass { class_ with c_self = super_pol; c_tparams = tparams })
    | Tfun fun_ ->
      let (acc, f_pc) = freshen_pol_con acc fun_.f_pc in
      let (acc, f_self) = freshen_pol_cov acc fun_.f_self in
      let (acc, f_args) = List.map_env acc ~f:freshen_con fun_.f_args in
      let (acc, f_ret) = freshen_cov acc fun_.f_ret in
      let (acc, f_exn) = freshen_cov acc fun_.f_exn in
      (acc, Tfun { f_pc; f_self; f_args; f_ret; f_exn })
  in
  let (acc, ty') = freshen adjustment env.e_acc ty in
  (Env.acc env (fun _ -> acc), ty')

let rec add_dependencies pl t acc =
  match t with
  | Tinter [] ->
    (* The policy p flows into a value of type mixed.
       Here we choose that mixed will be public; we
       could choose a different default policy or
       have mixed carry a policy (preferable). *)
    L.(pl <* [Pbot]) acc
  | Tprim pol
  | Tgeneric pol
  (* For classes and functions, we only add a dependency to the self policy and
     not to its properties *)
  | Tclass { c_self = pol; _ }
  | Tfun { f_self = pol; _ } ->
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

let rec class_ptype lump_pol_opt proto_renv targs name =
  let { cd_policied_properties } =
    match SMap.find_opt name proto_renv.pre_decl.de_class with
    | Some class_sig -> class_sig
    | None -> fail "could not found a class policy signature for %s" name
  in
  let prop_ptype { pp_name; pp_type; pp_purpose; _ } =
    (* Purpose of the property takes precedence over any lump policy. *)
    let lump_pol_opt =
      Option.merge
        (Option.map ~f:Lattice.parse_policy pp_purpose)
        lump_pol_opt
        ~f:(fun a _ -> a)
    in
    ( pp_name,
      lazy (ptype ~prefix:("." ^ pp_name) lump_pol_opt proto_renv pp_type) )
  in
  let lump_pol = get_policy lump_pol_opt proto_renv ~prefix:"lump" in
  let c_tparams =
    let prefix = "tp" in
    let ptype_tparam = ptype ~prefix lump_pol_opt proto_renv in
    let tparams = find_ancestor_tparams proto_renv name targs in
    let tparams =
      if List.is_empty targs then
        tparams
      else
        SMap.add name targs tparams
    in
    SMap.map (List.map ~f:ptype_tparam) tparams
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
  | T.Tgeneric (_name, _targs) ->
    (* TODO(T69551141) Handle type arguments *)
    Tgeneric (get_policy lump_pol_opt proto_renv ?prefix)
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
    let env = Env.acc env (subtype renv.re_proto.pre_meta pty new_pty) in
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

let throw renv env exn_ty =
  let union env t1 t2 = (env, mk_union t1 t2) in
  let env = Env.merge_conts_into ~union env [K.Next] K.Catch in
  let lpc = Env.get_lpc_policy env K.Next in
  Env.acc
    env
    L.(
      subtype renv.re_proto.pre_meta exn_ty renv.re_exn
      && add_dependencies (PCSet.elements lpc) renv.re_exn)

let call renv env callable_name that_pty_opt args_pty ret_ty =
  let ret_pty =
    let prefix = callable_name ^ "_ret" in
    ptype ~prefix None renv.re_proto ret_ty
  in
  let callee = Env.new_policy_var renv.re_proto callable_name in
  let callee_exn_policy = Env.new_policy_var renv.re_proto "exn" in
  let callee_exn =
    class_ptype (Some callee_exn_policy) renv.re_proto [] Decl.exception_id
  in
  let env =
    match SMap.find_opt callable_name renv.re_proto.pre_decl.de_fun with
    (* TODO(T68007489): Temporarily infer everything for every function call.
     * switch back to using InferFlows annotation when scaling is an issue.
     *)
    | Some _ ->
      (* The PC of the function being called depends on the join of the current
       * PC dependencies, as well as the function's own self policy *)
      let (env, pc_joined) =
        let join pc' (env, pc) = policy_join renv env ~prefix:"pcjoin" pc pc' in
        PCSet.fold join (Env.get_gpc_policy renv env K.Next) (env, callee)
      in
      let fp =
        {
          fp_name = callable_name;
          fp_this = that_pty_opt;
          fp_type =
            {
              f_pc = pc_joined;
              f_self = callee;
              f_args = args_pty;
              f_ret = ret_pty;
              f_exn = callee_exn;
            };
        }
      in
      let env = Env.acc env (fun acc -> Chole fp :: acc) in
      let env = Env.add_dep env callable_name in
      let env = Env.acc env @@ add_dependencies [callee] ret_pty in
      (* Any function call may throw, so we need to update the current PC and
       * exception dependencies based on the callee's exception policy
       *)
      let env = Env.push_pc env K.Next callee_exn_policy in
      throw renv env callee_exn
    | None -> fail "unknown function '%s'" callable_name
  in
  (env, ret_pty)

let vec_element_pty vec_pty =
  let class_ =
    match vec_pty with
    | Tclass cls when String.equal cls.c_name "\\HH\\vec" -> cls
    | _ -> fail "expected a vector"
  in
  match SMap.find_opt "\\HH\\vec" class_.c_tparams with
  | Some [element_pty] -> element_pty
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
    (env, prop_pty, Env.get_gpc_policy renv env K.Next)
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
    let meta = renv.re_proto.pre_meta in
    let env = Env.acc env @@ subtype meta old_vec_pty vec_pty in
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
  let env = Env.acc env (subtype renv.re_proto.pre_meta rhs_pty lhs_pty) in
  let env = Env.acc env (add_dependencies (PCSet.elements pc) lhs_pty) in
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
      adjust ~adjustment:Aweaken ~prefix:("." ^ property) renv env prop_ptype
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
      Env.acc env (subtype renv.re_proto.pre_meta pty element_pty)
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
  (* TODO(T70139741): support variadic functions and constructors
   * TODO(T70139893): support classes with type parameters
   *)
  | A.New (((_, lty), cid), _targs, args, _extra_args, _) ->
    let (env, args_pty) = List.map_env ~f:expr env args in
    let (env, obj_pty) =
      match cid with
      | A.CIexpr e -> expr env e
      | A.CI (_, _) -> (env, ptype None renv.re_proto lty)
      (* TODO(T70140005) support additional types of constructor calls *)
      | _ -> fail "could not find class name for constructor"
    in
    begin
      match obj_pty with
      | Tclass cls ->
        let callable_name =
          Decl.make_callable_name (Some cls.c_name) "__construct"
        in
        let lty = T.mk (Typing_reason.Rnone, T.Tprim A.Tvoid) in
        let (env, _) =
          call renv env callable_name (Some obj_pty) args_pty lty
        in
        let pty = ptype ?prefix:(Some "constr") None renv.re_proto ety in
        (env, pty)
      | _ -> fail "unhandled method call on %a" Pp.ptype obj_pty
    end
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
  let subtype = subtype renv.re_proto.pre_meta in
  match s with
  | A.Expr e ->
    let (env, _ty) = expr renv env e in
    env
  | A.If (e, b1, b2) ->
    let (env, ety) = expr renv env e in
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
        let (env, te) = expr renv env e in
        (* to account for enclosing conditionals, make the return
          type depend on the local pc *)
        let lpc = Env.get_lpc_policy env K.Next in
        Env.acc
          env
          L.(
            add_dependencies (PCSet.elements lpc) renv.re_ret
            && subtype te renv.re_ret)
    end
  | A.Throw e ->
    let (env, exn_ty) = expr renv env e in
    throw renv env exn_ty
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
      let fresh_exn = class_ptype None renv.re_proto [] Decl.exception_id in
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

let rec set_policy p = function
  | Tprim _ -> Tprim p
  | Tgeneric _ -> Tgeneric p
  | Ttuple l -> Ttuple (List.map ~f:(set_policy p) l)
  | Tunion l -> Tunion (List.map ~f:(set_policy p) l)
  | Tinter l -> Tinter (List.map ~f:(set_policy p) l)
  | Tclass c -> Tclass { c with c_self = p }
  | Tfun f ->
    Tfun
      {
        f_pc = p;
        f_self = p;
        f_args = List.map ~f:(set_policy p) f.f_args;
        f_ret = set_policy p f.f_ret;
        f_exn = set_policy p f.f_exn;
      }

let rec domain =
  let get_var = function
    | Pfree_var (v, s) -> VarSet.singleton (v, s)
    | _ -> VarSet.empty
  in
  let get_list =
    List.fold ~f:(fun s t -> VarSet.union s @@ domain t) ~init:VarSet.empty
  in
  function
  | Tprim p
  | Tgeneric p ->
    get_var p
  | Ttuple pl
  | Tunion pl
  | Tinter pl ->
    get_list pl
  | Tclass cls -> get_var cls.c_self
  | Tfun f ->
    f.f_ret :: f.f_exn :: f.f_args |> get_list |> VarSet.union (get_var f.f_pc)

(* Check if some function prototype is governed by a policy, meaning that the
 * policy flows into the argument and pc and that the return type flows into
 * the policy
 *)
let governs meta p fp acc =
  let proto_vars = domain @@ Tfun fp.fp_type in
  (* Quantify only the free variables that do not show up in the function
   * prototype. We eliminate them via simplification because they only show up
   * on one side of the entailment *)
  let quantify =
    let pred v = not @@ VarSet.mem v proto_vars in
    Logic.quantify ~pred ~quant:Qexists
  in
  let assumed_ty = set_policy p @@ Tfun fp.fp_type in
  let lattice =
    (* We assume that the prototype is equivalent to the assumed type. This
     * means that:
     *  proto <: assumed_ty : Policy P flows into input and out of outputs
     *  assumed_ty <: proto : Inputs flow into P and P flows into outputs
     * In the future, we can adjust these flows by using different assumed
     * types for the upper and lower bounds.
     *)
    equivalent meta (Tfun fp.fp_type) assumed_ty []
    |> Logic.conjoin
    |> quantify
    |> Logic.simplify
    |> Logic.flatten_prop
    |> Ifc_security_lattice.transitive_closure
  in
  let prop = quantify acc |> Logic.simplify in
  Logic.entailment_violations lattice prop

let callable meta decl_env class_name_opt name saved_env params body lrty =
  try
    (* Setup the read-only environment *)
    let scope = Scope.alloc () in
    let proto_renv = Env.new_proto_renv meta saved_env scope decl_env in

    let global_pc = Env.new_policy_var proto_renv "pc" in
    let exn = class_ptype None proto_renv [] Decl.exception_id in

    (* Here, we ignore the type parameters of this because at the moment we
     * lack Tgeneric policy type. This will be fixed (T68414656) in the future.
     *)
    let this_ty = Option.map class_name_opt (class_ptype None proto_renv []) in
    let ret_ty = ptype ~prefix:"ret" None proto_renv lrty in
    let renv = Env.new_renv proto_renv this_ty ret_ty global_pc exn in

    (* Initialise the mutable environment *)
    let env = Env.new_env in
    let (env, param_tys) = add_params renv env params in

    (* Run the analysis *)
    let beg_env = env in
    let env = block renv env body.A.fb_ast in
    let end_env = env in

    (* Display the analysis results *)
    if should_print meta.m_opts.opt_mode Manalyse then begin
      Format.printf "Analyzing %s:@." name;
      Format.printf "%a@." Pp.renv renv;
      Format.printf "* Params:@,  %a@." Pp.locals beg_env;
      Format.printf "* Final environment:@,  %a@." Pp.env end_env;
      Format.printf "@."
    end;

    let callable_name = Decl.make_callable_name class_name_opt name in
    let f_self = Env.new_policy_var proto_renv callable_name in

    let proto =
      {
        fp_name = Decl.make_callable_name class_name_opt name;
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
      | Some { fd_kind = FDCIPP } ->
        let implicit = Env.new_policy_var proto_renv "implicit" in
        governs meta implicit proto
      | _ -> const []
    in

    (* Return the results *)
    let res =
      {
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

let walk_tast meta decl_env =
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
        (callable meta decl_env None name saved_env params body lrty)
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
        let c_name = Some class_name in
        callable meta decl_env c_name name saved_env params body lrty
      in
      Some (List.filter_map ~f:handle_method methods)
    | _ -> None
  in
  (fun tast -> List.concat (List.filter_map ~f:def tast))

let opts_of_raw_opts raw_opts =
  let opt_mode =
    try Opts.parse_mode_exn raw_opts.ropt_mode
    with Opts.Invalid_ifc_mode mode ->
      fail "option error: %s is not a recognised mode" mode
  in
  let opt_security_lattice =
    try Lattice.mk_exn raw_opts.ropt_security_lattice
    with Lattice.Invalid_security_lattice ->
      fail
        "option error: lattice specification should be basic flux constraints, e.g., `A < B` separated by `;`"
  in
  { opt_mode; opt_security_lattice }

let do_ raw_opts files_info ctx =
  let opts = opts_of_raw_opts raw_opts in

  ( if should_print ~user_mode:opts.opt_mode ~phase:Mlattice then
    let lattice = opts.opt_security_lattice in
    Format.printf "@[Lattice:@. %a@]\n\n" Pp.security_lattice lattice );

  Relative_path.Map.iter files_info ~f:(fun path i ->
      (* skip decls and partial *)
      match i.FileInfo.file_mode with
      | Some FileInfo.Mstrict ->
        let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        let meta = { m_opts = opts; m_path = path; m_ctx = ctx } in
        let { Tast_provider.Compute_tast.tast; _ } =
          Tast_provider.compute_tast_unquarantined ~ctx ~entry
        in

        (* Declaration phase *)
        let decl_env = Decl.collect_sigs tast in
        if should_print ~user_mode:opts.opt_mode ~phase:Mdecl then
          Format.printf "%a@." Pp.decl_env decl_env;

        (* Flow analysis phase *)
        let results = walk_tast meta decl_env tast in

        (* Solver phase *)
        let results =
          try Solver.global_exn ~subtype:(subtype meta) results with
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
        let log_checking name (_, impl, simple) =
          let with_pp pp = List.map ~f:(fun v -> (v, pp)) in
          let violations =
            with_pp
              Pp.violation
              (Logic.entailment_violations opts.opt_security_lattice simple)
            @ with_pp Pp.implicit_violation impl
          in

          if
            should_print ~user_mode:opts.opt_mode ~phase:Mcheck
            && not (List.is_empty violations)
          then begin
            Format.printf "There are privacy policy errors in %s:@.  @[<v>" name;
            List.iter ~f:(fun (v, f) -> Format.printf "%a@," f v) violations;
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
class Cipp
 implements
 HH\FunctionAttribute,
 HH\MethodAttribute {}
|}
    );
  |]
