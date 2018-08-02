(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module used to declare the types.
 * For each class we want to build a complete type, that is the type of
 * the methods defined in the class plus everything that was inherited.
 *)
(*****************************************************************************)
open Hh_core
open Decl_defs
open Nast
open Typing_defs
open Typing_deps

module Reason = Typing_reason
module Inst = Decl_instantiate
module Attrs = Attributes

module SN = Naming_special_names

exception Decl_not_found of string

let conditionally_reactive_attribute_to_hint env { ua_params = l; _ } =
  match l with
  (* convert class const expression to non-generic type hint *)
  | [p, Class_const ((_, CI (cls, _)), (_, name))]
    when name = SN.Members.mClass ->
      (* set Extends dependency for between class that contains
         method and condition type *)
      Decl_env.add_extends_dependency env (snd cls);
      Decl_hint.hint env (p, Happly (cls, []))
  | _ ->
    (* error for invalid argument list was already reported during the
       naming step, do nothing *)
    Reason.none, Tany

let condition_type_from_attributes env user_attributes =
  Attributes.find SN.UserAttributes.uaOnlyRxIfImpl user_attributes
  |> Option.map ~f:(conditionally_reactive_attribute_to_hint env)

let fun_reactivity env user_attributes =
  let has attr = Attributes.mem attr user_attributes in
  let module UA = SN.UserAttributes in

  let rx_condition = condition_type_from_attributes env user_attributes in

  if has UA.uaReactive then Reactive rx_condition
  else if has UA.uaShallowReactive then Shallow rx_condition
  else if has UA.uaLocalReactive then Local rx_condition
  else Nonreactive

let adjust_reactivity_of_mayberx_parameter attrs reactivity param_ty =
  if Attributes.mem SN.UserAttributes.uaOnlyRxIfArgs attrs
  then make_function_type_mayberx reactivity param_ty
  else param_ty

(*****************************************************************************)
(* Checking that the kind of a class is compatible with its parent
 * For example, a class cannot extend an interface, an interface cannot
 * extend a trait etc ...
 *)
(*****************************************************************************)

let check_extend_kind parent_pos parent_kind child_pos child_kind =
  match parent_kind, child_kind with
    (* What is allowed *)
  | (Ast.Cabstract | Ast.Cnormal), (Ast.Cabstract | Ast.Cnormal)
  | Ast.Cabstract, Ast.Cenum (* enums extend BuiltinEnum under the hood *)
  | Ast.Ctrait, Ast.Ctrait
  | Ast.Cinterface, Ast.Cinterface ->
      ()
  | _ ->
      (* What is disallowed *)
      let parent = Ast.string_of_class_kind parent_kind in
      let child  = Ast.string_of_class_kind child_kind in
      Errors.wrong_extend_kind child_pos child parent_pos parent

let rec infer_const (p, expr_) = match expr_ with
  | String _ -> Reason.Rwitness p, Tprim Tstring
  | True
  | False -> Reason.Rwitness p, Tprim Tbool
  | Int _ -> Reason.Rwitness p, Tprim Tint
  | Float _ -> Reason.Rwitness p, Tprim Tfloat
  | Unop ((Ast.Uminus | Ast.Uplus | Ast.Utild | Ast.Unot), e2) ->
    infer_const e2
  | _ ->
    (* We can't infer the type of everything here. Notably, if you
     * define a const in terms of another const, we need an annotation,
     * since the other const may not have been declared yet.
     *
     * Also note that a number of expressions are considered invalid
     * as constant initializers, even if we can infer their type; see
     * Naming.check_constant_expr. *)
    raise Exit

let infer_const expr =
  try Some (infer_const expr) with Exit -> None

(*****************************************************************************)
(* Functions used retrieve everything implemented in parent classes
 * The return values:
 * env: the new environment
 * parents: the name of all the parents and grand parents of the class this
 *          includes traits.
 * is_complete: true if all the parents live in Hack
 *)
(*****************************************************************************)

let experimental_no_trait_reuse_enabled env =
  (TypecheckerOptions.experimental_feature_enabled
  env.Decl_env.decl_tcopt
  TypecheckerOptions.experimental_no_trait_reuse)

let report_reused_trait parent_type class_nast =
  Errors.trait_reuse parent_type.dc_pos parent_type.dc_name class_nast.c_name

(**
 * Verifies that a class never reuses the same trait throughout its hierarchy.
 *
 * Since Hack only has single inheritance and we already put up a warning for
 * cyclic class hierarchies, if there is any overlap between our extends and
 * our parents' extends, that overlap must be a trait.
 *
 * This does not hold for interfaces because they have multiple inheritance,
 * but interfaces cannot use traits in the first place.
 *
 * XHP attribute dependencies don't actually pull the trait into the class,
 * so we need to track them totally separately.
 *)
let check_no_duplicate_traits parent_type class_nast c_extends full_extends =
  let class_size = SSet.cardinal c_extends in
  let parents_size = SSet.cardinal parent_type.dc_extends in
  let full_size = SSet.cardinal full_extends in
  if (class_size + parents_size > full_size) then
    let duplicates = SSet.inter c_extends parent_type.dc_extends in
    SSet.iter (report_reused_trait parent_type class_nast) duplicates

(**
 * Adds the traits/classes which are part of a class' hierarchy.
 *
 * Traits are tracked separately but merged into the parents list when
 * typechecking so that the class can access the trait members which are
 * declared as private/protected.
 *)
let add_grand_parents_or_traits no_trait_reuse parent_pos class_nast acc parent_type =
  let extends, is_complete, pass = acc in
  let class_pos = fst class_nast.c_name in
  let class_kind = class_nast.c_kind in
  if pass = `Extends_pass then
    check_extend_kind parent_pos parent_type.dc_kind class_pos class_kind;
  (* If we are crawling the xhp attribute deps, we need to merge their xhp deps
   * as well *)
  let parent_deps = if pass = `Xhp_pass
    then SSet.union parent_type.dc_extends parent_type.dc_xhp_attr_deps
    else parent_type.dc_extends in
  let extends' = SSet.union extends parent_deps in
  (* Verify that merging the parent's extends did not introduce trait reuse *)
  if no_trait_reuse then
    check_no_duplicate_traits parent_type class_nast extends extends';
  extends', parent_type.dc_members_fully_known && is_complete, pass

let get_class_parent_or_trait env class_nast (parents, is_complete, pass)
    hint =
  (* See comment on check_no_duplicate_traits for reasoning here *)
  let no_trait_reuse = experimental_no_trait_reuse_enabled env
    && pass <> `Xhp_pass && class_nast.c_kind <> Ast.Cinterface
  in
  let parent_pos, parent, _ = Decl_utils.unwrap_class_hint hint in
  (* If we already had this exact trait, we need to flag trait reuse *)
  let reused_trait = no_trait_reuse && SSet.mem parent parents in
  let parents = SSet.add parent parents in
  let parent_type = Decl_env.get_class_dep env parent in
  match parent_type with
  | None ->
      (* The class lives in PHP *)
      parents, false, pass
  | Some parent_type ->
      (* The parent class lives in Hack, so we can report reused traits *)
      if reused_trait then report_reused_trait parent_type class_nast parent;
      let acc = parents, is_complete, pass in
      add_grand_parents_or_traits no_trait_reuse parent_pos class_nast acc parent_type

let get_class_parents_and_traits env class_nast =
  let parents = SSet.empty in
  let is_complete = true in
  (* extends parents *)
  let acc = parents, is_complete, `Extends_pass in
  let parents, is_complete, _ =
    List.fold_left class_nast.c_extends
      ~f:(get_class_parent_or_trait env class_nast) ~init:acc in
  (* traits *)
  let acc = parents, is_complete, `Traits_pass in
  let parents, is_complete, _ =
    List.fold_left class_nast.c_uses
      ~f:(get_class_parent_or_trait env class_nast) ~init:acc in
  (* XHP classes whose attributes were imported via "attribute :foo;" syntax *)
  let acc = SSet.empty, is_complete, `Xhp_pass in
  let xhp_parents, is_complete, _ =
    List.fold_left class_nast.c_xhp_attr_uses
      ~f:(get_class_parent_or_trait env class_nast) ~init:acc in
  parents, xhp_parents, is_complete

(*****************************************************************************)
(* Section declaring the type of a function *)
(*****************************************************************************)

let has_accept_disposable_attribute user_attributes =
  Attributes.mem SN.UserAttributes.uaAcceptDisposable user_attributes

let has_return_disposable_attribute user_attributes =
  Attributes.mem SN.UserAttributes.uaReturnDisposable user_attributes

let fun_returns_mutable user_attributes =
  Attributes.mem SN.UserAttributes.uaMutableReturn user_attributes

let fun_returns_void_to_rx user_attributes =
  Attributes.mem SN.UserAttributes.uaReturnsVoidToRx user_attributes

let get_param_mutability user_attributes =
  if Attributes.mem SN.UserAttributes.uaMutable user_attributes
  then Some Param_mutable
  else if Attributes.mem SN.UserAttributes.uaMaybeMutable user_attributes
  then Some Param_maybe_mutable
  else None

let rec ifun_decl tcopt (f: Ast.fun_) =
  let f = Errors.ignore_ (fun () -> Naming.fun_ tcopt f) in
  fun_decl f tcopt;
  ()

and make_param_ty env attrs reactivity param =
  let ty = match param.param_hint with
    | None ->
      let r = Reason.Rwitness param.param_pos in
      (r, Tany)
      (* if the code is strict, use the type-hint *)
    | Some x ->
      Decl_hint.hint env x
  in
  let ty = match ty with
    | _, t when param.param_is_variadic ->
      (* When checking a call f($a, $b) to a function f(C ...$args),
       * both $a and $b must be of type C *)
      Reason.Rvar_param param.param_pos, t
    | x -> x
  in
  let ty = adjust_reactivity_of_mayberx_parameter attrs reactivity ty in
  let mode = get_param_mode param.param_is_reference param.param_callconv in
  let rx_condition =
    if Attributes.mem SN.UserAttributes.uaOnlyRxIfRxFunc param.param_user_attributes
    then Some Param_rxfunc
    else
      Attributes.find SN.UserAttributes.uaOnlyRxIfImpl param.param_user_attributes
      |> Option.map ~f:(fun v -> Param_rx_if_impl (conditionally_reactive_attribute_to_hint env v))
    in
  {
    fp_pos  = param.param_pos;
    fp_name = Some param.param_name;
    fp_type = ty;
    fp_kind = mode;
    fp_mutability = get_param_mutability param.param_user_attributes;
    fp_accept_disposable =
      has_accept_disposable_attribute param.param_user_attributes;
    fp_rx_condition = rx_condition;
  }

and fun_decl f decl_tcopt =
  let errors, ft = Errors.do_ begin fun () ->
    let dep = Dep.Fun (snd f.f_name) in
    let env = {
      Decl_env.mode = f.f_mode;
      droot = Some dep;
      decl_tcopt;
    } in
    fun_decl_in_env env f
  end in
  let ft = { ft with ft_decl_errors = Some errors } in
  Decl_heap.Funs.add (snd f.f_name) ft;
  ()

and ret_from_fun_kind pos kind =
  let ty_any = (Reason.Rwitness pos, Tany) in
  match kind with
    | Ast.FGenerator ->
      let r = Reason.Rret_fun_kind (pos, kind) in
      r, Tapply ((pos, SN.Classes.cGenerator), [ty_any ; ty_any ; ty_any])
    | Ast.FAsyncGenerator ->
      let r = Reason.Rret_fun_kind (pos, kind) in
      r, Tapply ((pos, SN.Classes.cAsyncGenerator), [ty_any ; ty_any ; ty_any])
    | Ast.FAsync ->
      let r = Reason.Rret_fun_kind (pos, kind) in
      r, Tapply ((pos, SN.Classes.cAwaitable), [ty_any])
    | Ast.FSync
    | Ast.FCoroutine -> ty_any

and fun_decl_in_env env f =
  check_params env f.f_params;
  let reactivity = fun_reactivity env f.f_user_attributes in
  let returns_mutable = fun_returns_mutable f.f_user_attributes in
  let returns_void_to_rx = fun_returns_void_to_rx f.f_user_attributes in
  let return_disposable = has_return_disposable_attribute f.f_user_attributes in
  let arity_min = minimum_arity f.f_params in
  let params = make_params env f.f_user_attributes reactivity f.f_params in
  let ret_ty = match f.f_ret with
    | None -> ret_from_fun_kind (fst f.f_name) f.f_fun_kind
    | Some ty -> Decl_hint.hint env ty in
  let arity = match f.f_variadic with
    | FVvariadicArg param ->
      assert param.param_is_variadic;
      Fvariadic (arity_min, make_param_ty env f.f_user_attributes reactivity param)
    | FVellipsis p  -> Fellipsis (arity_min, p)
    | FVnonVariadic -> Fstandard (arity_min, List.length f.f_params)
  in
  let tparams = List.map f.f_tparams (type_param env) in
  let where_constraints =
    List.map f.f_where_constraints (where_constraint env) in
  let ft = {
    ft_pos         = fst f.f_name;
    ft_deprecated  =
      Attributes.deprecated ~kind:"function" f.f_name f.f_user_attributes;
    ft_is_coroutine = f.f_fun_kind = Ast.FCoroutine;
    ft_abstract    = false;
    ft_arity       = arity;
    ft_tparams     = tparams;
    ft_where_constraints = where_constraints;
    ft_params      = params;
    ft_ret         = ret_ty;
    ft_ret_by_ref  = f.f_ret_by_ref;
    ft_reactive    = reactivity;
    ft_mutability     = None; (* Functions can't be mutable because they don't have "this" *)
    ft_returns_mutable = returns_mutable;
    ft_return_disposable = return_disposable;
    ft_decl_errors = None;
    ft_returns_void_to_rx = returns_void_to_rx;
  } in
  ft

and type_param env (variance, x, cstrl, reified) =
  variance, x, List.map cstrl (fun (ck, h) -> (ck, Decl_hint.hint env h)), reified

and where_constraint env (ty1, ck, ty2) =
  (Decl_hint.hint env ty1, ck, Decl_hint.hint env ty2)

(* Functions building the types for the parameters of a function *)
(* It's not completely trivial because of optional arguments  *)

and minimum_arity paraml =
  (* We're looking for the minimum number of arguments that must be specified
  in a call to this method. Variadic "..." parameters need not be specified,
  parameters with default values need not be specified, so this method counts
  non-default-value, non-variadic parameters. *)
  let f param = (not param.param_is_variadic) && param.param_expr = None in
  List.count paraml f

and check_params env paraml =
  (* We wish to give an error on the first non-default parameter
  after a default parameter. That is:
  function foo(int $x, ?int $y = null, int $z)
  is an error on $z. *)
  (* TODO: This check doesn't need to be done at type checking time; it is
  entirely syntactic. When we switch over to the FFP, remove this code. *)
  let rec loop seen_default paraml =
    match paraml with
    | [] -> ()
    | param :: rl ->
        if param.param_is_variadic then
          () (* Assume that a variadic parameter is the last one we need
            to check. We've already given a parse error if the variadic
            parameter is not last. *)
        else if seen_default && param.param_expr = None then
          Errors.previous_default param.param_pos
          (* We've seen at least one required parameter, and there's an
          optional parameter after it.  Given an error, and then stop looking
          for more errors in this parameter list. *)
        else
          loop (param.param_expr <> None) rl
  in
  (* PHP allows non-default valued parameters after default valued parameters. *)
  if (env.Decl_env.mode <> FileInfo.Mphp) then
    loop false paraml

and make_params env attrs reactivity paraml =
  List.map paraml ~f:(make_param_ty env attrs reactivity)

(*****************************************************************************)
(* Section declaring the type of a class *)
(*****************************************************************************)

type class_env = {
  tcopt: TypecheckerOptions.t;
  stack: SSet.t;
}

let check_if_cyclic class_env (pos, cid) =
  let stack = class_env.stack in
  let is_cyclic = SSet.mem cid stack in
  if is_cyclic
  then Errors.cyclic_class_def stack pos;
  is_cyclic

let rec class_decl_if_missing class_env c =
  let _, cid as c_name = c.Ast.c_name in
  if check_if_cyclic class_env c_name
  then ()
  else begin
    if Decl_heap.Classes.mem cid then () else
      (* Class elements are in memory if and only if the class itself is there.
       * Exiting before class declaration is ready would break this invariant *)
      WorkerCancel.with_no_cancellations @@ fun () ->
      class_naming_and_decl class_env cid c
  end

and class_naming_and_decl (class_env:class_env) cid c =
  let class_env = { class_env with stack = SSet.add cid class_env.stack } in
  let c = Errors.ignore_ (fun () -> Naming.class_ class_env.tcopt c) in
  let errors, tc = Errors.do_ begin fun() ->
    class_parents_decl class_env c;
    class_decl class_env.tcopt c
  end in
  Decl_heap.Classes.add (snd c.c_name) { tc with dc_decl_errors = Some errors };
  ()

and class_parents_decl class_env c =
  let class_hint = class_hint_decl class_env in
  List.iter c.c_extends class_hint;
  List.iter c.c_implements class_hint;
  List.iter c.c_uses class_hint;
  List.iter c.c_xhp_attr_uses class_hint;
  List.iter c.c_req_extends class_hint;
  List.iter c.c_req_implements class_hint;
  ()

and is_disposable_type env hint =
  match hint with
  | (_, Happly ((_, c), _)) ->
    begin match Decl_env.get_class_dep env c with
    | None -> false
    | Some c -> c.dc_is_disposable
    end
  | _ -> false

and class_hint_decl class_env hint =
  match hint with
  | _, Happly ((_, cid), _) ->
    begin match Naming_heap.TypeIdHeap.get cid with
      | Some (pos, `Class) when not (Decl_heap.Classes.mem cid) ->
        let fn = FileInfo.get_pos_filename pos in
        (* We are supposed to redeclare the class *)
        let class_opt = Parser_heap.find_class_in_file class_env.tcopt fn cid in
        Errors.run_in_context fn Errors.Decl begin fun () ->
          Option.iter class_opt (class_decl_if_missing class_env)
        end
      | _ -> ()
    end
  | _ ->
    (* This class lives in PHP land *)
    ()

and class_is_abstract c =
  match c.c_kind with
    | Ast.Cabstract | Ast.Cinterface | Ast.Ctrait | Ast.Cenum -> true
    | _ -> false

and class_decl tcopt c =
  let is_abstract = class_is_abstract c in
  let const = Attrs.mem SN.UserAttributes.uaConst c.c_user_attributes in
  let is_ppl = Attrs.mem SN.UserAttributes.uaProbabilisticModel c.c_user_attributes in
  let _p, cls_name = c.c_name in
  let class_dep = Dep.Class cls_name in
  let env = {
    Decl_env.mode = c.c_mode;
    droot = Some class_dep;
    decl_tcopt = tcopt;
  } in
  let inherited = Decl_inherit.make env c in
  let props = inherited.Decl_inherit.ih_props in
  let props =
    List.fold_left ~f:(class_var_decl env c) ~init:props c.c_vars in
  let m = inherited.Decl_inherit.ih_methods in
  let m, condition_types = List.fold_left
      ~f:(method_decl_acc ~is_static:false env c )
      ~init:(m, SSet.empty) c.c_methods in
  let consts = inherited.Decl_inherit.ih_consts in
  let consts = List.fold_left ~f:(class_const_decl env c)
    ~init:consts c.c_consts in
  let consts = SMap.add SN.Members.mClass (class_class_decl c.c_name) consts in
  let typeconsts = inherited.Decl_inherit.ih_typeconsts in
  let typeconsts, consts = List.fold_left c.c_typeconsts
      ~f:(typeconst_decl env c) ~init:(typeconsts, consts) in
  let sclass_var = static_class_var_decl env c in
  let sprops = inherited.Decl_inherit.ih_sprops in
  let sprops = List.fold_left c.c_static_vars ~f:sclass_var ~init:sprops in
  let sm = inherited.Decl_inherit.ih_smethods in
  let sm, condition_types = List.fold_left c.c_static_methods
      ~f:(method_decl_acc ~is_static:true env c )
      ~init:(sm, condition_types) in
  let parent_cstr = inherited.Decl_inherit.ih_cstr in
  let cstr = constructor_decl env parent_cstr c in
  let has_concrete_cstr = match (fst cstr) with
    | None
    | Some {elt_abstract = true; _} -> false
    | _ -> true in
  let impl = c.c_extends @ c.c_implements @ c.c_uses in
  let impl = List.map impl (Decl_hint.hint env) in
  let impl = match SMap.get SN.Members.__toString m with
    | Some { elt_origin = cls; _} when cls_name <> SN.Classes.cStringish ->
      (* HHVM implicitly adds Stringish interface for every class/iface/trait
       * with a __toString method; "string" also implements this interface *)
      let pos = method_pos tcopt ~is_static:false cls SN.Members.__toString  in
      let hint = pos, Happly ((pos, SN.Classes.cStringish), []) in
      (* Declare Stringish and parents if not already declared *)
      let class_env = { tcopt; stack = SSet.empty } in
      class_hint_decl class_env hint;
      let ty = (Reason.Rhint pos, Tapply ((pos, SN.Classes.cStringish), [])) in
      ty :: impl
    | _ -> impl
  in
  let impl = List.map impl (get_implements env) in
  let impl = List.fold_right impl ~f:(SMap.fold SMap.add) ~init:SMap.empty in
  let extends, xhp_attr_deps, ext_strict = get_class_parents_and_traits env c in
  let req_ancestors, req_ancestors_extends =
    Decl_requirements.get_class_requirements env c in
  (* Interfaces IDisposable and IAsyncDisposable are *disposable types*, as
   * are any classes that implement either of these interfaces, directly or
   * indirectly. Also treat any trait that *requires* extension or
   * implementation of a disposable class as disposable itself.
   *)
  let is_disposable_class_name cls_name =
    cls_name = SN.Classes.cIDisposable ||
    cls_name = SN.Classes.cIAsyncDisposable in
  let is_disposable =
    is_disposable_class_name cls_name ||
    SMap.exists (fun n _ -> is_disposable_class_name n) impl ||
    List.exists (c.c_req_extends @ c.c_req_implements) (is_disposable_type env) in
  (* If this class is disposable then we require that any extended class or
   * trait that is used, is also disposable, in order that escape analysis
   * has been applied on the $this parameter.
   *)
  let ext_strict = List.fold_left c.c_uses
    ~f:(trait_exists env) ~init:ext_strict in
  if not ext_strict &&
      (env.Decl_env.mode = FileInfo.Mstrict) then
    let p, name = c.c_name in
    Errors.strict_members_not_known p name
  else ();
  let tparams = List.map (fst c.c_tparams) (type_param env) in
  let enum = match c.c_enum with
    | None -> None
    | Some e ->
      let base_hint = Decl_hint.hint env e.e_base in
      let constraint_hint =
        Option.map e.e_constraint (Decl_hint.hint env) in
      Some
        { te_base       = base_hint;
          te_constraint = constraint_hint } in
  let consts = Decl_enum.rewrite_class c.c_name enum impl consts in
  let has_own_cstr = has_concrete_cstr && (None <> c.c_constructor) in
  let deferred_members = Decl_init_check.class_ ~has_own_cstr env c in
  let sealed_whitelist = get_sealed_whitelist c in
  let tc = {
    dc_final = c.c_final;
    dc_const = const;
    dc_ppl = is_ppl;
    dc_abstract = is_abstract;
    dc_need_init = has_concrete_cstr;
    dc_deferred_init_members = deferred_members;
    dc_members_fully_known = ext_strict;
    dc_kind = c.c_kind;
    dc_is_xhp = c.c_is_xhp;
    dc_is_disposable = is_disposable;
    dc_name = snd c.c_name;
    dc_pos = fst c.c_name;
    dc_tparams = tparams;
    dc_substs = inherited.Decl_inherit.ih_substs;
    dc_consts = consts;
    dc_typeconsts = typeconsts;
    dc_props = props;
    dc_sprops = sprops;
    dc_methods = m;
    dc_smethods = sm;
    dc_construct = cstr;
    dc_ancestors = impl;
    dc_extends = extends;
    dc_sealed_whitelist = sealed_whitelist;
    dc_xhp_attr_deps = xhp_attr_deps;
    dc_req_ancestors = req_ancestors;
    dc_req_ancestors_extends = req_ancestors_extends;
    dc_enum_type = enum;
    dc_decl_errors = None;
    dc_condition_types = condition_types;
  } in
  if Ast.Cnormal = c.c_kind then
    begin
      SMap.iter (
        method_check_trait_overrides tcopt ~is_static:false c
      ) m;
      SMap.iter (
        method_check_trait_overrides tcopt ~is_static:true c
      ) sm;
    end
  else ();
  SMap.iter begin fun x _ ->
    Typing_deps.add_idep class_dep (Dep.Class x)
  end impl;
  tc

and get_sealed_whitelist c =
  match Attrs.find SN.UserAttributes.uaSealed c.c_user_attributes with
    | None -> None
    | Some {ua_params = params; _} ->
      begin match c.c_kind with
        | Ast.Cenum ->
          let pos = fst c.c_name in
          let kind = String.capitalize_ascii (Ast.string_of_class_kind c.c_kind) in
          Errors.unsealable pos kind;
          None
        | Ast.Cabstract | Ast.Cinterface | Ast.Cnormal | Ast.Ctrait ->
          let p, name = c.c_name in
          if c.c_final then Errors.sealed_final p name;
          let add_class_name names param =
            match param with
              | _, Class_const ((_, CI cls), (_, name))
                when name = SN.Members.mClass ->
                SSet.add (get_instantiated_sid_name cls) names
              | _ -> names in
          Some (List.fold_left params ~f:add_class_name ~init:SSet.empty)
      end

and get_implements env ht =
  let _r, (_p, c), paraml = Decl_utils.unwrap_class_type ht in
  let class_ = Decl_env.get_class_dep env c in
  match class_ with
  | None ->
      (* The class lives in PHP land *)
      SMap.singleton c ht
  | Some class_ ->
      let subst = Inst.make_subst class_.dc_tparams paraml in
      let sub_implements =
        SMap.map
          (fun ty -> Inst.instantiate subst ty)
          class_.dc_ancestors
      in
      SMap.add c ht sub_implements

and trait_exists env acc trait =
  match trait with
    | (_, Happly ((_, trait), _)) ->
      let class_ = Decl_env.get_class_dep env trait in
      (match class_ with
        | None -> false
        | Some _class -> acc
      )
    | _ -> false

and constructor_decl env (pcstr, pconsist) class_ =
  (* constructors in children of class_ must be consistent? *)
  let cconsist = class_.c_final ||
    Attrs.mem
      SN.UserAttributes.uaConsistentConstruct
      class_.c_user_attributes in
  match class_.c_constructor, pcstr with
  | None, _ -> pcstr, cconsist || pconsist
  | Some method_, Some {elt_final = true; elt_origin; _ } ->
    let ft = Decl_heap.Constructors.find_unsafe elt_origin in
    Errors.override_final ~parent:(ft.ft_pos) ~child:(fst method_.m_name);
    let cstr, mconsist = build_constructor env class_ method_ in
    cstr, cconsist || mconsist || pconsist
  | Some method_, _ ->
    let cstr, mconsist = build_constructor env class_ method_ in
    cstr, cconsist || mconsist || pconsist

and build_constructor env class_ method_ =
  let ft = method_decl env method_ in
  let _, class_name = class_.c_name in
  let vis = visibility class_name method_.m_visibility in
  let mconsist = method_.m_final || class_.c_kind == Ast.Cinterface in
  (* due to the requirement of calling parent::__construct, a private
   * constructor cannot be overridden *)
  let mconsist = mconsist || method_.m_visibility == Private in
  let mconsist = mconsist || ft.ft_abstract in
  (* the alternative to overriding
   * UserAttributes.uaConsistentConstruct is marking the corresponding
   * 'new static()' UNSAFE, potentially impacting the safety of a large
   * type hierarchy. *)
  let consist_override =
    Attrs.mem SN.UserAttributes.uaUnsafeConstruct method_.m_user_attributes in
  let cstr = {
    elt_final = method_.m_final;
    elt_abstract = ft.ft_abstract;
    elt_is_xhp_attr = false;
    elt_const = false;
    elt_override = consist_override;
    elt_synthesized = false;
    elt_visibility = vis;
    elt_origin = class_name;
    elt_reactivity = None;
  } in
  Decl_heap.Constructors.add class_name ft;
  Some cstr, mconsist

and class_const_decl env c acc (h, id, e) =
  match c.c_kind with
  | Ast.Ctrait ->
      let kind = match c.c_kind with
        | Ast.Ctrait -> `trait
        | Ast.Cenum -> `enum
        | _ -> assert false in
      Errors.cannot_declare_constant kind (fst id) c.c_name;
      acc
  | Ast.Cnormal | Ast.Cabstract | Ast.Cinterface | Ast.Cenum ->
    let c_name = (snd c.c_name) in
    let ty, abstract =
      (* Optional hint h, optional expression e *)
      match h, e with
      | Some h, Some _ ->
        Decl_hint.hint env h, false
      | Some h, None ->
        Decl_hint.hint env h, true
      | None, Some e ->
          begin match infer_const e with
            | Some ty -> ty, false
            | None ->
              if c.c_mode = FileInfo.Mstrict && c.c_kind <> Ast.Cenum
              then Errors.missing_typehint (fst id);
              (Reason.Rwitness (fst id), Tany), false
          end
        | None, None ->
          let pos, _name = id in
          if c.c_mode = FileInfo.Mstrict then Errors.missing_typehint pos;
          let r = Reason.Rwitness pos in
          (r, Tany), true
    in
    let cc = {
      cc_synthesized = false;
      cc_abstract = abstract;
      cc_pos = fst id;
      cc_type = ty;
      cc_expr = e;
      cc_origin = c_name;
    } in
    let acc = SMap.add (snd id) cc acc in
    acc

(* Every class, interface, and trait implicitly defines a ::class to
 * allow accessing its fully qualified name as a string *)
and class_class_decl class_id =
  let pos, name = class_id in
  let reason = Reason.Rclass_class (pos, name) in
  let classname_ty =
    reason, Tapply ((pos, SN.Classes.cClassname), [reason, Tthis]) in
  {
    cc_abstract    = false;
    cc_pos         = pos;
    cc_synthesized = true;
    cc_type        = classname_ty;
    cc_expr        = None;
    cc_origin      = name;
  }

and class_var_decl env c acc cv =
  let ty = match cv.cv_type with
    | None -> Reason.Rwitness (fst cv.cv_id), Tany
    | Some ty' when cv.cv_is_xhp ->
      (* If this is an XHP attribute and we're in strict mode,
         relax to partial mode to allow the use of the "array"
         annotation without specifying type parameters. Until
         recently HHVM did not allow "array" with type parameters
         in XHP attribute declarations, so this is a temporary
         hack to support existing code for now. *)
      (* Task #5815945: Get rid of this Hack *)
      let env = if Decl_env.mode env = FileInfo.Mstrict
      then { env with Decl_env.mode = FileInfo.Mpartial }
        else env
      in
      Decl_hint.hint env ty'
    | Some ty' -> Decl_hint.hint env ty'
  in
  let id = snd cv.cv_id in
  let vis = visibility (snd c.c_name) cv.cv_visibility in
  let const = Attrs.mem SN.UserAttributes.uaConst cv.cv_user_attributes in
  let elt = {
    elt_final = true;
    elt_is_xhp_attr = cv.cv_is_xhp;
    elt_const = const;
    elt_synthesized = false;
    elt_override = false;
    elt_abstract = false;
    elt_visibility = vis;
    elt_origin = (snd c.c_name);
    elt_reactivity = None;
  } in
  Decl_heap.Props.add (elt.elt_origin, id) ty;
  let acc = SMap.add id elt acc in
  if cv.cv_final then Errors.final_property (fst cv.cv_id);
  acc

and static_class_var_decl env c acc cv =
  let ty = match cv.cv_type with
    | None -> Reason.Rwitness (fst cv.cv_id), Tany
    | Some ty -> Decl_hint.hint env ty in
  let id = "$" ^ snd cv.cv_id in
  let vis = visibility (snd c.c_name) cv.cv_visibility in
  let elt = {
    elt_final = true;
    elt_const = false; (* unsupported for static properties *)
    elt_is_xhp_attr = cv.cv_is_xhp;
    elt_override = false;
    elt_abstract = false;
    elt_synthesized = false;
    elt_visibility = vis;
    elt_origin = (snd c.c_name);
    elt_reactivity = None;
  } in
  Decl_heap.StaticProps.add (elt.elt_origin, id) ty;
  let acc = SMap.add id elt acc in
  if cv.cv_expr = None && FileInfo.(c.c_mode = Mstrict || c.c_mode = Mpartial)
  then begin match cv.cv_type with
    | None
    | Some (_, Hmixed)
    | Some (_, Hoption _) -> ()
    | _ -> Errors.missing_assign (fst cv.cv_id)
  end;
  acc

and visibility cid = function
  | Public    -> Vpublic
  | Protected -> Vprotected cid
  | Private   -> Vprivate cid

(* each concrete type constant T = <sometype> implicitly defines a
class constant with the same name which is TypeStructure<sometype> *)
and typeconst_ty_decl pos dc_name ~is_abstract =
  let r = Reason.Rwitness pos in
  let tsid = pos, SN.FB.cTypeStructure in
  let ts_ty = r, Tapply (tsid, [r, Taccess ((r, Tthis), [pos, dc_name])]) in
  {
    cc_abstract    = is_abstract;
    cc_pos         = pos;
    cc_synthesized = true;
    cc_type        = ts_ty;
    cc_expr        = None;
    cc_origin      = dc_name;
  }

and typeconst_decl env c (acc, acc2) {
  c_tconst_name = (pos, name);
  c_tconst_constraint = constr;
  c_tconst_type = type_;
} =
  match c.c_kind with
  | Ast.Ctrait | Ast.Cenum ->
      let kind = match c.c_kind with
        | Ast.Ctrait -> `trait
        | Ast.Cenum -> `enum
        | _ -> assert false in
      Errors.cannot_declare_constant kind pos c.c_name;
      acc, acc2
  | Ast.Cinterface | Ast.Cabstract | Ast.Cnormal ->
      let constr = Option.map constr (Decl_hint.hint env) in
      let ty = Option.map type_ (Decl_hint.hint env) in
      let is_abstract = Option.is_none ty in
      let ts = typeconst_ty_decl pos name ~is_abstract in
      let acc2 = SMap.add name ts acc2 in
      let tc = {
        ttc_name = (pos, name);
        ttc_constraint = constr;
        ttc_type = ty;
        ttc_origin = snd c.c_name;
      } in
      let acc = SMap.add name tc acc in
      acc, acc2

and method_decl env m =
  check_params env m.m_params;
  let reactivity = fun_reactivity env m.m_user_attributes in
  let mut = get_param_mutability m.m_user_attributes in
  let returns_mutable = fun_returns_mutable m.m_user_attributes in
  let returns_void_to_rx = fun_returns_void_to_rx m.m_user_attributes in
  let return_disposable = has_return_disposable_attribute m.m_user_attributes in
  let arity_min = minimum_arity m.m_params in
  let params = make_params env m.m_user_attributes reactivity m.m_params in
  let ret = match m.m_ret with
    | None -> ret_from_fun_kind (fst m.m_name) m.m_fun_kind
    | Some ret -> Decl_hint.hint env ret in
  let arity = match m.m_variadic with
    | FVvariadicArg param ->
      assert param.param_is_variadic;
      assert (param.param_expr = None);
      Fvariadic (arity_min, make_param_ty env m.m_user_attributes reactivity param)
    | FVellipsis p  -> Fellipsis (arity_min, p)
    | FVnonVariadic -> Fstandard (arity_min, List.length m.m_params)
  in
  let tparams = List.map m.m_tparams (type_param env) in
  let where_constraints =
    List.map m.m_where_constraints (where_constraint env) in
  {
    ft_pos      = fst m.m_name;
    ft_deprecated =
      Attrs.deprecated ~kind:"method" m.m_name m.m_user_attributes;
    ft_abstract = m.m_abstract;
    ft_is_coroutine = m.m_fun_kind = Ast.FCoroutine;
    ft_arity    = arity;
    ft_tparams  = tparams;
    ft_where_constraints = where_constraints;
    ft_params   = params;
    ft_ret      = ret;
    ft_ret_by_ref = m.m_ret_by_ref;
    ft_reactive = reactivity;
    ft_mutability = mut;
    ft_returns_mutable = returns_mutable;
    ft_return_disposable = return_disposable;
    ft_decl_errors = None;
    ft_returns_void_to_rx = returns_void_to_rx;
  }

and method_check_override c m acc  =
  let pos, id = m.m_name in
  let _, class_id = c.c_name in
  let override = Attrs.mem SN.UserAttributes.uaOverride m.m_user_attributes in
  if m.m_visibility = Private && override then
    Errors.private_override pos class_id id;
  match SMap.get id acc with
  | Some _ -> false (* overriding final methods is handled in typing *)
  | None when override && c.c_kind = Ast.Ctrait -> true
  | None when override ->
    Errors.should_be_override pos class_id id;
    false
  | None -> false

and method_decl_acc ~is_static env c (acc, condition_types) m  =
  let check_override = method_check_override c m acc in
  let ft = method_decl env m in
  let _, id = m.m_name in
  let condition_types, reactivity =
    match ft.ft_reactive with
    | Reactive (Some (_, Tapply ((_, cls), []))) ->
      SSet.add cls condition_types, Some (Decl_defs.Method_reactive (Some cls))
    | Reactive None ->
      condition_types, Some (Decl_defs.Method_reactive None)
    | Shallow (Some (_, Tapply ((_, cls), []))) ->
      SSet.add cls condition_types, Some (Decl_defs.Method_shallow (Some cls))
    | Shallow None ->
      condition_types, Some (Decl_defs.Method_shallow None)
    | Local (Some (_, Tapply ((_, cls), [])))  ->
      SSet.add cls condition_types, Some (Decl_defs.Method_local (Some cls))
    | Local None ->
      condition_types, Some (Decl_defs.Method_local None)
    | _ -> condition_types, None
  in
  let vis =
    match SMap.get id acc, m.m_visibility with
    | Some { elt_visibility = Vprotected _ as parent_vis; _ }, Protected ->
      parent_vis
    | _ -> visibility (snd c.c_name) m.m_visibility
  in
  let elt = {
    elt_final = m.m_final;
    elt_is_xhp_attr = false;
    elt_const = false;
    elt_abstract = ft.ft_abstract;
    elt_override = check_override;
    elt_synthesized = false;
    elt_visibility = vis;
    elt_origin = snd (c.c_name);
    elt_reactivity = reactivity;
  } in
  let add_meth = if is_static
    then Decl_heap.StaticMethods.add
    else Decl_heap.Methods.add
  in
  add_meth (elt.elt_origin, id) ft;
  let acc = SMap.add id elt acc in
  acc, condition_types

and method_check_trait_overrides opt ~is_static c id meth =
  if meth.elt_override then begin
    let pos = method_pos opt ~is_static meth.elt_origin id in
    Errors.override_per_trait c.c_name id pos
  end

(* For the most part the declaration phase does not care about the position of
 * methods. There are a few places that do mainly for error reporting. There
 * are cases when the method has not been added to the Decl_heap yet, in which
 * case we fallback to retrieving the position from the parsing AST.
 *)
and method_pos opt ~is_static class_id meth  =
  let get_meth = if is_static then
      Decl_heap.StaticMethods.get
    else
    Decl_heap.Methods.get in
  match get_meth (class_id, meth) with
  | Some { ft_pos; _ } -> ft_pos
  | None ->
    try
      match Naming_heap.TypeIdHeap.get class_id with
      | Some (pos, `Class) ->
        let fn = FileInfo.get_pos_filename pos in
        begin match Parser_heap.find_class_in_file opt fn class_id with
          | None -> raise Not_found
          | Some { Ast.c_body; _ } ->
            let elt = List.find ~f:begin fun x ->
              match x with
              | Ast.Method {Ast.m_name = (_, name); m_kind; _ }
                when name = meth && is_static = List.mem m_kind Ast.Static ->
                true
              | _ -> false
            end c_body
            in
            begin match elt with
              | Some (Ast.Method { Ast.m_name = (pos, _); _ }) -> pos
              | _ -> raise Not_found
            end
        end
      | _ -> raise Not_found
    with
    | Not_found -> Pos.none


(*****************************************************************************)
(* Dealing with typedefs *)
(*****************************************************************************)

let rec type_typedef_decl_if_missing tcopt typedef =
  let _, tid = typedef.Ast.t_id in
  if Decl_heap.Typedefs.mem tid
  then ()
  else
    type_typedef_naming_and_decl tcopt typedef

and typedef_decl tdef decl_tcopt =
  let {
    t_annotation = ();
    t_name = td_pos, tid;
    t_tparams = params;
    t_constraint = tcstr;
    t_kind = concrete_type;
    t_user_attributes = _;
    t_mode = mode;
    t_vis = td_vis;
  } = tdef in
  let dep = Typing_deps.Dep.Class tid in
  let env = {Decl_env.mode = mode; droot = Some dep; decl_tcopt} in
  let td_tparams = List.map params (type_param env) in
  let td_type = Decl_hint.hint env concrete_type in
  let td_constraint = Option.map tcstr (Decl_hint.hint env) in
  let td_decl_errors = None in
  {
    td_vis; td_tparams; td_constraint; td_type; td_pos; td_decl_errors;
  }

and type_typedef_naming_and_decl tcopt tdef =
  let tdef = Errors.ignore_ (fun () -> Naming.typedef tcopt tdef) in
  let errors, tdecl = Errors.do_ (fun () -> typedef_decl tdef tcopt) in
  Decl_heap.Typedefs.add (snd tdef.t_name)
    { tdecl with td_decl_errors = Some errors};
  ()

(*****************************************************************************)
(* Global constants *)
(*****************************************************************************)

let const_decl cst decl_tcopt =
  let open Option.Monad_infix in
  let cst_pos, _cst_name = cst.cst_name in
  let dep = Dep.GConst (snd cst.cst_name) in
  let env = {Decl_env.mode = cst.cst_mode; droot = Some dep; decl_tcopt} in
  match cst.cst_type with
  | Some h -> Decl_hint.hint env h
  | None ->
    match cst.cst_value >>= infer_const with
    | Some ty -> ty
    | None when cst.cst_mode = FileInfo.Mstrict && (not cst.cst_is_define) ->
      Errors.missing_typehint cst_pos;
      Reason.Rwitness cst_pos, Tany
    | None ->
      Reason.Rwitness cst_pos, Tany

let iconst_decl tcopt cst =
  let cst = Errors.ignore_ (fun () -> Naming.global_const tcopt cst) in
  let errors, hint_ty = Errors.do_ (fun() -> const_decl cst tcopt) in
  Decl_heap.GConsts.add (snd cst.cst_name) (hint_ty, errors);
  ()

(*****************************************************************************)

let rec name_and_declare_types_program tcopt prog =
  List.iter prog begin fun def ->
    match def with
    | Ast.Namespace (_, prog) -> name_and_declare_types_program tcopt prog
    | Ast.NamespaceUse _ -> ()
    | Ast.SetNamespaceEnv _ -> ()
    | Ast.Fun f -> ifun_decl tcopt f
    | Ast.Class c ->
      let class_env = {
        tcopt;
        stack = SSet.empty;
      } in
      class_decl_if_missing class_env c
    | Ast.Typedef typedef ->
      type_typedef_decl_if_missing tcopt typedef
    | Ast.Stmt _ -> ()
    | Ast.Constant cst -> iconst_decl tcopt cst
  end

let make_env tcopt fn =
  let ast = Parser_heap.get_from_parser_heap tcopt fn in
  name_and_declare_types_program tcopt ast

let err_not_found file name =
  let err_str =
    Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file) in
raise (Decl_not_found err_str)

let declare_class_in_file tcopt file name =
  match Parser_heap.find_class_in_file tcopt file name with
  | Some cls ->
    let class_env = { tcopt; stack = SSet.empty; } in
    class_decl_if_missing class_env cls
  | None ->
    err_not_found file name

let declare_fun_in_file tcopt file name =
  match Parser_heap.find_fun_in_file tcopt file name with
  | Some f -> ifun_decl tcopt f
  | None ->
    err_not_found file name

let declare_typedef_in_file tcopt file name =
  match Parser_heap.find_typedef_in_file tcopt file name with
  | Some t -> type_typedef_naming_and_decl tcopt t
  | None ->
    err_not_found file name

let declare_const_in_file tcopt file name =
  match Parser_heap.find_const_in_file tcopt file name with
  | Some cst -> iconst_decl tcopt cst
  | None ->
    err_not_found file name
