(* Copyright (c) 2020, Facebook, Inc.
   All rights reserved. *)
open Hh_prelude
open Hh_core
open Ifc_types
module Env = Ifc_env
module Logic = Ifc_logic
module Decl = Ifc_decl
module Pp = Ifc_pretty
module A = Aast
module T = Typing_defs
module L = Logic.Infix
module Reason = Typing_reason

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
    let policied_prop_pol_zip =
      match
        List.zip
          (SMap.values cl1.c_property_map)
          (SMap.values cl2.c_property_map)
      with
      | Some zip -> zip
      | None -> fail "same class with differing policied properties"
    in
    (* Invariant in property policies *)
    (* When forcing an unevaluated ptype thunk, only policied properties
     * will be populated. This invariant, and the ban on recursive class cycles
     * through policied properties (enforcement still to be done, see T68078692)
     * ensures termination here.
     *)
    List.fold policied_prop_pol_zip ~init:acc ~f:(fun acc (t1, t2) ->
        equivalent (Lazy.force t1) (Lazy.force t2) acc)
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
let weaken ?prefix renv env ty =
  (* Prefix to use for policy variables generated through weakning *)
  let pvar_prefix = Option.value prefix ~default:"weak" in
  let rec freshen acc ty =
    let on_list mk tl =
      let (acc, tl') = List.map_env acc tl ~f:freshen in
      (acc, mk tl')
    in
    match ty with
    | Tprim p ->
      let p' = Env.new_policy_var renv.re_proto pvar_prefix in
      (L.(p < p') acc, Tprim p')
    | Ttuple tl -> on_list (fun l -> Ttuple l) tl
    | Tunion tl -> on_list (fun l -> Tunion l) tl
    | Tinter tl -> on_list (fun l -> Tinter l) tl
    | Tclass class_ ->
      let super_pol = Env.new_policy_var renv.re_proto pvar_prefix in
      let acc = L.(class_.c_self < super_pol) acc in
      (acc, Tclass { class_ with c_self = super_pol })
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

let policy_join renv env p1 p2 =
  (* Prefix to use for policy variables generated through policy joins *)
  let pvar_prefix = "join" in
  match Logic.policy_join p1 p2 with
  | Some p -> (env, p)
  | None ->
    (match (p1, p2) with
    | (Pfree_var (v1, s1), Pfree_var (v2, s2))
      when equal_policy_var v1 v2 && Scope.equal s1 s2 ->
      (env, p1)
    | _ ->
      let pv = Env.new_policy_var renv.re_proto pvar_prefix in
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
let get_policy_var ?prefix lump_pol_opt proto_renv =
  match lump_pol_opt with
  | Some lump_pol -> lump_pol
  | None ->
    let prefix = Option.value prefix ~default:"v" in
    Env.new_policy_var proto_renv prefix

let rec class_ptype lump_pol_opt proto_renv name =
  let { psig_policied_properties } =
    match SMap.find_opt name proto_renv.pre_psig_env with
    | Some class_policy_sig -> class_policy_sig
    | None -> fail "could not found a class policy signature for %s" name
  in
  let policied_props =
    List.map psig_policied_properties (fun (prop, ty) ->
        (prop, lazy (ptype ~prefix:("." ^ prop) lump_pol_opt proto_renv ty)))
  in
  let lump_pol = get_policy_var lump_pol_opt proto_renv ~prefix:"lump" in
  Tclass
    {
      c_name = name;
      c_self = get_policy_var lump_pol_opt proto_renv ~prefix:name;
      c_lump = lump_pol;
      c_property_map = SMap.of_list policied_props;
    }

(* Turns a locl_ty into a type with policy annotations;
   the policy annotations are fresh policy variables *)
and ptype ?prefix lump_pol_opt proto_renv (t : T.locl_ty) =
  let ptype = ptype ?prefix lump_pol_opt proto_renv in
  match T.get_node t with
  | T.Tprim _ -> Tprim (get_policy_var lump_pol_opt proto_renv ?prefix)
  | T.Ttuple tyl -> Ttuple (List.map ~f:ptype tyl)
  | T.Tunion tyl -> Tunion (List.map ~f:ptype tyl)
  | T.Tintersection tyl -> Tinter (List.map ~f:ptype tyl)
  | T.Tclass ((_, name), _, _) -> class_ptype lump_pol_opt proto_renv name
  | T.Tvar id ->
    (* Drops the environment `expand_var` returns. This is logically
     * correct, but threading the envrionment would lead to faster future
     * `Tvar` lookups.
     *)
    let (_, ty) =
      Typing_inference_env.expand_var
        proto_renv.pre_tenv.Tast.inference_env
        Reason.Rnone
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

let add_params renv env params =
  let add_param env p =
    let prefix = p.A.param_name in
    let pty = ptype None renv.re_proto (fst p.A.param_type_hint) ~prefix in
    let lid = Local_id.make_unscoped p.A.param_name in
    Env.set_local_type env lid pty
  in
  List.fold params ~init:env ~f:add_param

type lvalue =
  | Local of A.local_id
  | Property of ptype

let binop renv env ty1 ty2 =
  match (ty1, ty2) with
  | (Tprim p1, Tprim p2) ->
    let (env, pj) = policy_join renv env p1 p2 in
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

let rec lvalue renv env (((_epos, ety), e) : Tast.expr) =
  match e with
  | A.Lvar (_pos, lid) -> (env, Local lid)
  | A.Obj_get (obj, (_, A.Id (_, property)), _) ->
    let (env, obj_ptype) = expr renv env obj in
    let obj_pol = (receiver_of_obj_get obj_ptype property).c_self in
    let prop_ptype = property_ptype renv.re_proto obj_ptype property ety in
    let env = Env.acc env (add_dependencies [obj_pol] prop_ptype) in
    (env, Property prop_ptype)
  | _ -> fail "unsupported lvalue"

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
  | A.Binop (Ast_defs.Eq op, e1, e2) ->
    let (env, tyo) = lvalue renv env e1 in
    let (env, ty) = expr env e2 in
    let env =
      match tyo with
      | Local lid ->
        let (env, ty) =
          if Option.is_none op then
            (env, ty)
          else
            let lty = Env.get_local_type env lid in
            binop renv env lty ty
        in
        let prefix = Local_id.to_string lid in
        let (env, ty) = weaken renv env ty ~prefix in
        let env = Env.acc env (add_dependencies renv.re_lpc ty) in
        Env.set_local_type env lid ty
      | Property prop_ptype ->
        let (env, ty) =
          if Option.is_none op then
            (env, ty)
          else
            binop renv env prop_ptype ty
        in
        let env = Env.acc env (subtype ty prop_ptype) in
        Env.acc env (add_dependencies renv.re_gpc prop_ptype)
    in
    (env, ty)
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
  | A.Array_get (_, _)
  | A.Class_get (_, _)
  | A.Class_const (_, _)
  | A.Call (_, _, _, _, _)
  | A.FunctionPointer (_, _)
  | A.String2 _
  | A.PrefixedString (_, _)
  | A.Yield _
  | A.Yield_break
  | A.Yield_from _
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
  | A.Record (_, _, _)
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
    let renv' =
      let epol =
        match ety with
        | Tprim p -> p
        | _ -> fail "condition expression must be of type bool"
      in
      { renv with re_lpc = epol :: renv.re_lpc; re_gpc = epol :: renv.re_gpc }
    in
    let cenv = Env.get_cenv env in
    let env = block renv' (Env.set_cenv env cenv) b1 in
    let cenv1 = Env.get_cenv env in
    let env = block renv' (Env.set_cenv env cenv) b2 in
    let cenv2 = Env.get_cenv env in
    let union _env t1 t2 = (env, mk_union t1 t2) in
    Env.merge_and_set_cenv ~union env cenv1 cenv2
  | A.Return (Some e) ->
    let (env, te) = expr renv env e in
    Env.acc env (subtype te renv.re_ret)
  | A.Return None -> env
  | _ -> env

and block renv env (blk : Tast.block) =
  List.fold_left ~f:(stmt renv) ~init:env blk

let func_or_method psig_env class_name_opt name saved_env params body lrty =
  begin
    try
      (* Setup the read-only environment *)
      let scope = Scope.alloc () in
      let proto_renv = Env.new_proto_renv saved_env scope psig_env in

      let global_pc = Env.new_policy_var proto_renv "pc" in
      let this_ty = Option.map class_name_opt (class_ptype None proto_renv) in
      let ret_ty = ptype ~prefix:"ret" None proto_renv lrty in
      let renv = Env.new_renv proto_renv global_pc this_ty ret_ty in

      (* Initialise the mutable environment *)
      let env = Env.new_env in
      let env = add_params renv env params in

      (* Run the analysis *)
      let beg_env = env in
      let env = block renv env body.A.fb_ast in
      let end_env = env in

      (* Display the analysis results *)
      Format.printf "Analyzing %s:@." name;
      Format.printf "%a@." Pp.renv renv;
      Format.printf "* @[<hov2>Params:@ %a@]@." Pp.locals beg_env;
      Format.printf "* Final environment:@,  %a@." Pp.env end_env
    with FlowInference s -> Format.printf "  Failure: %s@." s
  end;
  Format.printf "@."

let walk_tast psig_env =
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
      func_or_method psig_env None name saved_env params body lrty
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
        func_or_method
          psig_env
          (Some class_name)
          name
          saved_env
          params
          body
          lrty
      in
      List.iter methods ~f:handle_method
    | _ -> ()
  in
  List.iter ~f:def

let do_ files_info opts ctx =
  Relative_path.Map.iter files_info ~f:(fun path i ->
      (* skip decls and partial *)
      match i.FileInfo.file_mode with
      | Some FileInfo.Mstrict ->
        let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        let { Tast_provider.Compute_tast.tast; _ } =
          Tast_provider.compute_tast_unquarantined ~ctx ~entry
        in
        let psig_env = Decl.collect_class_policy_sigs tast in
        Format.printf "%a" Pp.policy_sig_env psig_env;

        if String.equal opts "prtast" then
          Format.printf "TAST: %a@." Tast.pp_program tast;

        walk_tast psig_env tast
      | _ -> ());
  ()

let magic_builtins =
  [|
    ( "ifc_magic.hhi",
      {|<?hh // strict
class Policied implements HH\InstancePropertyAttribute, HH\ClassAttribute { }
|}
    );
  |]
