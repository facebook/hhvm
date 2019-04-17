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
open Core_kernel
open Decl_defs
open Decl_fun_utils
open Nast
open Shallow_decl_defs
open Typing_defs
open Typing_deps

module Reason = Typing_reason
module Inst = Decl_instantiate
module Attrs = Attributes

module SN = Naming_special_names

let tracked_names = ref None

let start_tracking () =
  tracked_names := Some FileInfo.empty_names

let record_fun s = match !tracked_names with
  | None -> ()
  | Some names -> tracked_names := Some FileInfo.{ names with
      n_funs = SSet.add s names.n_funs
  }

let record_class s = match !tracked_names with
  | None -> ()
  | Some names -> tracked_names := Some FileInfo.{ names with
      n_classes = SSet.add s names.n_classes
  }

let record_typedef s = match !tracked_names with
  | None -> ()
  | Some names -> tracked_names := Some FileInfo.{ names with
      n_types = SSet.add s names.n_types
  }

let record_const s = match !tracked_names with
  | None -> ()
  | Some names -> tracked_names := Some FileInfo.{ names with
      n_consts = SSet.add s names.n_consts
  }

let stop_tracking () =
  let res = match !tracked_names with
    | None ->
      Hh_logger.log "Warning: called Decl.stop_tracking without corresponding start_tracking";
      FileInfo.empty_names
    | Some names -> names
  in
  tracked_names := None;
  res

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

let report_reused_trait parent_type shallow_class =
  Errors.trait_reuse parent_type.dc_pos parent_type.dc_name shallow_class.sc_name

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
let check_no_duplicate_traits parent_type shallow_class c_extends full_extends =
  let class_size = SSet.cardinal c_extends in
  let parents_size = SSet.cardinal parent_type.dc_extends in
  let full_size = SSet.cardinal full_extends in
  if (class_size + parents_size > full_size) then
    let duplicates = SSet.inter c_extends parent_type.dc_extends in
    SSet.iter (report_reused_trait parent_type shallow_class) duplicates

(**
 * Adds the traits/classes which are part of a class' hierarchy.
 *
 * Traits are tracked separately but merged into the parents list when
 * typechecking so that the class can access the trait members which are
 * declared as private/protected.
 *)
let add_grand_parents_or_traits no_trait_reuse parent_pos shallow_class acc parent_type =
  let extends, is_complete, pass = acc in
  let class_pos = fst shallow_class.sc_name in
  let class_kind = shallow_class.sc_kind in
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
    check_no_duplicate_traits parent_type shallow_class extends extends';
  extends', parent_type.dc_members_fully_known && is_complete, pass

let get_class_parent_or_trait env shallow_class (parents, is_complete, pass) ty =
  (* See comment on check_no_duplicate_traits for reasoning here *)
  let no_trait_reuse = experimental_no_trait_reuse_enabled env
    && pass <> `Xhp_pass && shallow_class.sc_kind <> Ast.Cinterface
  in
  let _, (parent_pos, parent), _ = Decl_utils.unwrap_class_type ty in
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
      if reused_trait then report_reused_trait parent_type shallow_class parent;
      let acc = parents, is_complete, pass in
      add_grand_parents_or_traits no_trait_reuse parent_pos shallow_class acc parent_type

let get_class_parents_and_traits env shallow_class =
  let parents = SSet.empty in
  let is_complete = true in
  (* extends parents *)
  let acc = parents, is_complete, `Extends_pass in
  let parents, is_complete, _ =
    List.fold_left shallow_class.sc_extends
      ~f:(get_class_parent_or_trait env shallow_class) ~init:acc in
  (* traits *)
  let acc = parents, is_complete, `Traits_pass in
  let parents, is_complete, _ =
    List.fold_left shallow_class.sc_uses
      ~f:(get_class_parent_or_trait env shallow_class) ~init:acc in
  (* XHP classes whose attributes were imported via "attribute :foo;" syntax *)
  let acc = SSet.empty, is_complete, `Xhp_pass in
  let xhp_parents, is_complete, _ =
    List.fold_left shallow_class.sc_xhp_attr_uses
      ~f:(get_class_parent_or_trait env shallow_class) ~init:acc in
  parents, xhp_parents, is_complete

(*****************************************************************************)
(* Section declaring the type of a function *)
(*****************************************************************************)

let rec ifun_decl (f: Ast.fun_) =
  let f = Errors.ignore_ (fun () -> Naming.fun_ f) in
  fun_decl f;
  ()

and fun_decl f =
  let errors, ft = Errors.do_ begin fun () ->
    let dep = Dep.Fun (snd f.f_name) in
    let env = {
      Decl_env.mode = f.f_mode;
      droot = Some dep;
      decl_tcopt = GlobalNamingOptions.get ();
    } in
    fun_decl_in_env env f
  end in
  let ft = { ft with ft_decl_errors = Some errors } in
  record_fun (snd f.f_name);
  Decl_heap.Funs.add (snd f.f_name) ft;
  ()

and fun_decl_in_env env f =
  check_params env f.f_params;
  let reactivity = fun_reactivity env f.f_user_attributes in
  let returns_mutable = fun_returns_mutable f.f_user_attributes in
  let returns_void_to_rx = fun_returns_void_to_rx f.f_user_attributes in
  let return_disposable = has_return_disposable_attribute f.f_user_attributes in
  let arity_min = minimum_arity f.f_params in
  let params = make_params env f.f_params in
  let ret_ty = match f.f_ret with
    | None -> ret_from_fun_kind (fst f.f_name) f.f_fun_kind
    | Some ty -> Decl_hint.hint env ty in
  let arity = match f.f_variadic with
    | FVvariadicArg param ->
      assert param.param_is_variadic;
      Fvariadic (arity_min, make_param_ty env param)
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
    ft_tparams     = (tparams, FTKtparams);
    ft_where_constraints = where_constraints;
    ft_params      = params;
    ft_ret         = ret_ty;
    ft_reactive    = reactivity;
    ft_mutability     = None; (* Functions can't be mutable because they don't have "this" *)
    ft_returns_mutable = returns_mutable;
    ft_return_disposable = return_disposable;
    ft_decl_errors = None;
    ft_returns_void_to_rx = returns_void_to_rx;
  } in
  ft

(*****************************************************************************)
(* Section declaring the type of a class *)
(*****************************************************************************)

type class_env = {
  stack: SSet.t;
}

let check_if_cyclic class_env (pos, cid) =
  let stack = class_env.stack in
  let is_cyclic = SSet.mem cid stack in
  if is_cyclic
  then Errors.cyclic_class_def stack pos;
  is_cyclic

let shallow_decl_enabled () =
  TypecheckerOptions.shallow_class_decl (GlobalNamingOptions.get ())

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
  let class_env = { stack = SSet.add cid class_env.stack } in
  let shallow_class =
    Shallow_classes_heap.class_naming_and_decl c in
  let errors, tc = Errors.do_ begin fun() ->
    class_parents_decl class_env shallow_class;
    class_decl shallow_class
  end in
  let errors = Errors.merge shallow_class.sc_decl_errors errors in
  let name = snd shallow_class.sc_name in
  record_class name;
  Decl_heap.Classes.add name { tc with dc_decl_errors = Some errors };
  ()

and class_parents_decl class_env c =
  let class_type = class_type_decl class_env in
  List.iter c.sc_extends class_type;
  List.iter c.sc_implements class_type;
  List.iter c.sc_uses class_type;
  List.iter c.sc_xhp_attr_uses class_type;
  List.iter c.sc_req_extends class_type;
  List.iter c.sc_req_implements class_type;
  ()

and is_disposable_type env hint =
  match hint with
  | (_, Tapply ((_, c), _)) ->
    begin match Decl_env.get_class_dep env c with
    | None -> false
    | Some c -> c.dc_is_disposable
    end
  | _ -> false

and class_type_decl class_env hint =
  match hint with
  | _, Tapply ((_, cid), _) ->
    begin match Naming_table.Types.get_pos cid with
      | Some (pos, Naming_table.TClass) when not (Decl_heap.Classes.mem cid) ->
        let fn = FileInfo.get_pos_filename pos in
        (* We are supposed to redeclare the class *)
        let class_opt = Parser_heap.find_class_in_file fn cid in
        Errors.run_in_context fn Errors.Decl begin fun () ->
          Option.iter class_opt (class_decl_if_missing class_env)
        end
      | _ -> ()
    end
  | _ ->
    (* This class lives in PHP land *)
    ()

and class_is_abstract c =
  match c.sc_kind with
    | Ast.Cabstract | Ast.Cinterface | Ast.Ctrait | Ast.Cenum -> true
    | _ -> false

and class_decl c =
  let is_abstract = class_is_abstract c in
  let const = Attrs.mem SN.UserAttributes.uaConst c.sc_user_attributes in
  let is_ppl = Attrs.mem SN.UserAttributes.uaProbabilisticModel c.sc_user_attributes in
  let _p, cls_name = c.sc_name in
  let class_dep = Dep.Class cls_name in
  let env = {
    Decl_env.mode = c.sc_mode;
    droot = Some class_dep;
    decl_tcopt = GlobalNamingOptions.get ();
  } in
  let inherited = Decl_inherit.make env c in
  let props = inherited.Decl_inherit.ih_props in
  let props = List.fold_left ~f:(prop_decl c) ~init:props c.sc_props in
  let redecl_smethods, redecl_methods =
    List.partition_tf ~f:(fun x -> x.smr_static) c.sc_method_redeclarations in
  let m = inherited.Decl_inherit.ih_methods in
  let m = List.fold_left ~f:(method_redecl_acc c) ~init:m redecl_methods in
  let m, condition_types = List.fold_left
      ~f:(method_decl_acc ~is_static:false c )
      ~init:(m, SSet.empty) c.sc_methods in
  let consts = inherited.Decl_inherit.ih_consts in
  let consts = List.fold_left ~f:(class_const_fold c)
    ~init:consts c.sc_consts in
  let consts = SMap.add SN.Members.mClass (class_class_decl c.sc_name) consts in
  let typeconsts = inherited.Decl_inherit.ih_typeconsts in
  let typeconsts, consts = List.fold_left c.sc_typeconsts
      ~f:(typeconst_fold c) ~init:(typeconsts, consts) in
  let sclass_var = static_prop_decl c in
  let sprops = inherited.Decl_inherit.ih_sprops in
  let sprops = List.fold_left c.sc_sprops ~f:sclass_var ~init:sprops in
  let sm = inherited.Decl_inherit.ih_smethods in
  let sm = List.fold_left ~f:(method_redecl_acc c) ~init:sm redecl_smethods in
  let sm, condition_types = List.fold_left c.sc_static_methods
      ~f:(method_decl_acc ~is_static:true c )
      ~init:(sm, condition_types) in
  let parent_cstr = inherited.Decl_inherit.ih_cstr in
  let cstr = constructor_decl parent_cstr c in
  let has_concrete_cstr = match (fst cstr) with
    | None
    | Some {elt_abstract = true; _} -> false
    | _ -> true in
  let impl = c.sc_extends @ c.sc_implements @ c.sc_uses in
  let impl = match SMap.get SN.Members.__toString m with
    | Some { elt_origin = cls; _} when cls_name <> SN.Classes.cStringish ->
      (* HHVM implicitly adds Stringish interface for every class/iface/trait
       * with a __toString method; "string" also implements this interface *)
      let pos = method_pos ~is_static:false cls SN.Members.__toString  in
      (* Declare Stringish and parents if not already declared *)
      let class_env = { stack = SSet.empty } in
      let ty = (Reason.Rhint pos, Tapply ((pos, SN.Classes.cStringish), [])) in
      class_type_decl class_env ty;
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
    List.exists (c.sc_req_extends @ c.sc_req_implements) (is_disposable_type env) in
  (* If this class is disposable then we require that any extended class or
   * trait that is used, is also disposable, in order that escape analysis
   * has been applied on the $this parameter.
   *)
  let ext_strict = List.fold_left c.sc_uses
    ~f:(trait_exists env) ~init:ext_strict in
  if not ext_strict &&
     (FileInfo.is_strict env.Decl_env.mode) then
    let p, name = c.sc_name in
    Errors.strict_members_not_known p name
  else ();
  let enum = c.sc_enum_type in
  let consts =
    Decl_enum.rewrite_class c.sc_name enum (fun x -> SMap.get x impl) consts in
  let has_own_cstr = has_concrete_cstr && (None <> c.sc_constructor) in
  let deferred_members =
    if shallow_decl_enabled ()
    then SSet.empty
    else Decl_init_check.class_ ~has_own_cstr env c
  in
  let sealed_whitelist = get_sealed_whitelist c in
  let tc = {
    dc_final = c.sc_final;
    dc_const = const;
    dc_ppl = is_ppl;
    dc_abstract = is_abstract;
    dc_need_init = has_concrete_cstr;
    dc_deferred_init_members = deferred_members;
    dc_members_fully_known = ext_strict;
    dc_kind = c.sc_kind;
    dc_is_xhp = c.sc_is_xhp;
    dc_is_disposable = is_disposable;
    dc_name = snd c.sc_name;
    dc_pos = fst c.sc_name;
    dc_tparams = c.sc_tparams;
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
  if Ast.Cnormal = c.sc_kind then
    begin
      SMap.iter (
        method_check_trait_overrides ~is_static:false c
      ) m;
      SMap.iter (
        method_check_trait_overrides ~is_static:true c
      ) sm;
    end
  else ();
  SMap.iter begin fun x _ ->
    Typing_deps.add_idep class_dep (Dep.Class x)
  end impl;
  tc

and get_sealed_whitelist c =
  match Attrs.find SN.UserAttributes.uaSealed c.sc_user_attributes with
    | None -> None
    | Some {ua_params = params; _} ->
        let add_class_name names param =
          match param with
            | _, Class_const ((_, CI cls), (_, name))
              when name = SN.Members.mClass ->
              SSet.add (snd cls) names
            | _ -> names in
        Some (List.fold_left params ~f:add_class_name ~init:SSet.empty)

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
    | (_, Tapply ((_, trait), _)) ->
      let class_ = Decl_env.get_class_dep env trait in
      (match class_ with
        | None -> false
        | Some _class -> acc
      )
    | _ -> false

and constructor_decl (pcstr, pconsist) class_ =
  (* constructors in children of class_ must be consistent? *)
  let cconsist =
    if class_.sc_final
    then FinalClass
    else if Attrs.mem SN.UserAttributes.uaConsistentConstruct class_.sc_user_attributes
      then ConsistentConstruct
      else Inconsistent in

  let cstr = match class_.sc_constructor, pcstr with
    | None, _ -> pcstr
    | Some method_, Some {elt_final = true; elt_origin; _ } ->
      let ft = Decl_heap.Constructors.find_unsafe elt_origin in
      Errors.override_final ~parent:(ft.ft_pos) ~child:(fst method_.sm_name);
      let cstr = build_constructor class_ method_ in
      cstr
    | Some method_, _ ->
      let cstr = build_constructor class_ method_ in
      cstr in
  cstr, Decl_utils.coalesce_consistent pconsist cconsist

and build_constructor class_ method_ =
  let ft = method_.sm_type in
  let _, class_name = class_.sc_name in
  let vis = visibility class_name method_.sm_visibility in
  let cstr = {
    elt_final = method_.sm_final;
    elt_abstract = ft.ft_abstract;
    elt_is_xhp_attr = false;
    elt_const = false;
    elt_lateinit = false;
    elt_lsb = false;
    elt_override = false;
    elt_memoizelsb = false;
    elt_synthesized = false;
    elt_visibility = vis;
    elt_origin = class_name;
    elt_reactivity = None;
  } in
  Decl_heap.Constructors.add class_name ft;
  Some cstr

and class_const_fold c acc scc =
  let c_name = (snd c.sc_name) in
  let cc = {
    cc_synthesized = false;
    cc_abstract = scc.scc_abstract;
    cc_pos = fst scc.scc_name;
    cc_type = scc.scc_type;
    cc_expr = scc.scc_expr;
    cc_origin = c_name;
  } in
  let acc = SMap.add (snd scc.scc_name) cc acc in
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

and prop_decl c acc sp =
  let sp_pos, sp_name = sp.sp_name in
  let ty = match sp.sp_type with
    | None -> Reason.Rwitness sp_pos, Tany
    | Some ty' -> ty'
  in
  let vis = visibility (snd c.sc_name) sp.sp_visibility in
  let elt = {
    elt_final = true;
    elt_is_xhp_attr = sp.sp_is_xhp_attr;
    elt_const = sp.sp_const;
    elt_lateinit = sp.sp_lateinit;
    elt_lsb = false;
    elt_synthesized = false;
    elt_override = false;
    elt_memoizelsb = false;
    elt_abstract = false;
    elt_visibility = vis;
    elt_origin = (snd c.sc_name);
    elt_reactivity = None;
  } in
  Decl_heap.Props.add (elt.elt_origin, sp_name) ty;
  let acc = SMap.add sp_name elt acc in
  acc

and static_prop_decl c acc sp =
  let sp_pos, sp_name = sp.sp_name in
  let ty = match sp.sp_type with
    | None -> Reason.Rwitness sp_pos, Tany
    | Some ty' -> ty'
  in
  let vis = visibility (snd c.sc_name) sp.sp_visibility in
  let elt = {
    elt_final = true;
    elt_const = false; (* unsupported for static properties *)
    elt_lateinit = sp.sp_lateinit;
    elt_lsb = sp.sp_lsb;
    elt_is_xhp_attr = sp.sp_is_xhp_attr;
    elt_override = false;
    elt_memoizelsb = false;
    elt_abstract = false;
    elt_synthesized = false;
    elt_visibility = vis;
    elt_origin = (snd c.sc_name);
    elt_reactivity = None;
  } in
  Decl_heap.StaticProps.add (elt.elt_origin, sp_name) ty;
  let acc = SMap.add sp_name elt acc in
  acc

and visibility cid = function
  | Public    -> Vpublic
  | Protected -> Vprotected cid
  | Private   -> Vprivate cid

(* each concrete type constant T = <sometype> implicitly defines a
class constant with the same name which is TypeStructure<sometype> *)
and typeconst_structure c stc =
  let pos = fst stc.stc_name in
  let r = Reason.Rwitness pos in
  let tsid = pos, SN.FB.cTypeStructure in
  let ts_ty = r, Tapply (tsid, [r, Taccess ((r, Tthis), [stc.stc_name])]) in
  {
    cc_abstract    = Option.is_none stc.stc_type;
    cc_pos         = pos;
    cc_synthesized = true;
    cc_type        = ts_ty;
    cc_expr        = None;
    cc_origin      = snd c.sc_name;
  }

and typeconst_fold c ((typeconsts, consts) as acc) stc =
  match c.sc_kind with
  | Ast.Ctrait | Ast.Cenum | Ast.Crecord -> acc
  | Ast.Cinterface | Ast.Cabstract | Ast.Cnormal ->
    let name = (snd stc.stc_name) in
    let c_name = (snd c.sc_name) in
    let ts = typeconst_structure c stc in
    let consts = SMap.add name ts consts in
    let enforceable =
      (* Without the positions, this is a simple OR, but this way allows us to
       * report the position of the <<__Enforceable>> attribute to the user *)
      if snd stc.stc_enforceable
      then stc.stc_enforceable
      else match SMap.get name typeconsts with
        | Some ptc -> ptc.ttc_enforceable
        | None -> Pos.none, false in
    let tc = {
      ttc_abstract = stc.stc_abstract;
      ttc_name = stc.stc_name;
      ttc_constraint = stc.stc_constraint;
      ttc_type = stc.stc_type;
      ttc_origin = c_name;
      ttc_enforceable = enforceable;
    } in
    let typeconsts = SMap.add (snd stc.stc_name) tc typeconsts in
    typeconsts, consts

and method_check_override c m acc  =
  let pos, id = m.sm_name in
  let _, class_id = c.sc_name in
  let override = m.sm_override in
  match SMap.get id acc with
  | Some _ -> false (* overriding final methods is handled in typing *)
  | None when override && c.sc_kind = Ast.Ctrait -> true
  | None when override ->
    Errors.should_be_override pos class_id id;
    false
  | None -> false

and method_redecl_acc c acc m =
  let ft = m.smr_type in
  let _, id = m.smr_name in
  let vis =
    match SMap.get id acc, m.smr_visibility with
    | Some { elt_visibility = Vprotected _ as parent_vis; _ }, Protected ->
      parent_vis
    | _ -> visibility (snd c.sc_name) m.smr_visibility
  in
  let elt = {
    elt_final = m.smr_final;
    elt_is_xhp_attr = false;
    elt_const = false;
    elt_lateinit = false;
    elt_lsb = false;
    elt_abstract = ft.ft_abstract;
    elt_override = false;
    elt_memoizelsb = false;
    elt_synthesized = false;
    elt_visibility = vis;
    elt_origin = snd (c.sc_name);
    elt_reactivity = None;
  } in
  let add_meth = if m.smr_static
    then Decl_heap.StaticMethods.add
    else Decl_heap.Methods.add
  in
  add_meth (elt.elt_origin, id) ft;
  SMap.add id elt acc

and method_decl_acc ~is_static c (acc, condition_types) m  =
  let check_override = method_check_override c m acc in
  let has_memoizelsb = m.sm_memoizelsb in
  let ft = m.sm_type in
  let _, id = m.sm_name in
  let condition_types =
    match ft.ft_reactive with
    | Reactive (Some (_, Tapply ((_, cls), []))) ->
      SSet.add cls condition_types
    | Reactive None ->
      condition_types
    | Shallow (Some (_, Tapply ((_, cls), []))) ->
      SSet.add cls condition_types
    | Shallow None ->
      condition_types
    | Local (Some (_, Tapply ((_, cls), [])))  ->
      SSet.add cls condition_types
    | Local None ->
      condition_types
    | _ -> condition_types
  in
  let vis =
    match SMap.get id acc, m.sm_visibility with
    | Some { elt_visibility = Vprotected _ as parent_vis; _ }, Protected ->
      parent_vis
    | _ -> visibility (snd c.sc_name) m.sm_visibility
  in
  let elt = {
    elt_final = m.sm_final;
    elt_is_xhp_attr = false;
    elt_const = false;
    elt_lateinit = false;
    elt_lsb = false;
    elt_abstract = ft.ft_abstract;
    elt_override = check_override;
    elt_memoizelsb = has_memoizelsb;
    elt_synthesized = false;
    elt_visibility = vis;
    elt_origin = snd (c.sc_name);
    elt_reactivity = m.sm_reactivity;
  } in
  let add_meth = if is_static
    then Decl_heap.StaticMethods.add
    else Decl_heap.Methods.add
  in
  add_meth (elt.elt_origin, id) ft;
  let acc = SMap.add id elt acc in
  acc, condition_types

and method_check_trait_overrides ~is_static c id meth =
  if meth.elt_override then begin
    let pos = method_pos ~is_static meth.elt_origin id in
    Errors.override_per_trait c.sc_name id pos
  end

(* For the most part the declaration phase does not care about the position of
 * methods. There are a few places that do mainly for error reporting. There
 * are cases when the method has not been added to the Decl_heap yet, in which
 * case we fallback to retrieving the position from the parsing AST.
 *)
and method_pos ~is_static class_id meth  =
  let get_meth = if is_static then
      Decl_heap.StaticMethods.get
    else
    Decl_heap.Methods.get in
  match get_meth (class_id, meth) with
  | Some { ft_pos; _ } -> ft_pos
  | None ->
    try
      match Naming_table.Types.get_pos class_id with
      | Some (pos, Naming_table.TClass) ->
        let fn = FileInfo.get_pos_filename pos in
        begin match Parser_heap.find_class_in_file fn class_id with
          | None -> raise Caml.Not_found
          | Some { Ast.c_body; _ } ->
            let elt = List.find ~f:begin fun x ->
              match x with
              | Ast.Method {Ast.m_name = (_, name); m_kind; _ }
                when name = meth && is_static = List.mem m_kind Ast.Static ~equal:(=) ->
                true
              | _ -> false
            end c_body
            in
            begin match elt with
              | Some (Ast.Method { Ast.m_name = (pos, _); _ }) -> pos
              | _ -> raise Caml.Not_found
            end
        end
      | _ -> raise Caml.Not_found
    with
    | Caml.Not_found -> Pos.none


(*****************************************************************************)
(* Dealing with typedefs *)
(*****************************************************************************)

let rec type_typedef_decl_if_missing typedef =
  let _, tid = typedef.Ast.t_id in
  if Decl_heap.Typedefs.mem tid
  then ()
  else
    type_typedef_naming_and_decl typedef

and typedef_decl tdef =
  let {
    t_annotation = ();
    t_name = td_pos, tid;
    t_tparams = params;
    t_constraint = tcstr;
    t_kind = concrete_type;
    t_user_attributes = _;
    t_namespace = _;
    t_mode = mode;
    t_vis = td_vis;
  } = tdef in
  let dep = Typing_deps.Dep.Class tid in
  let env = {Decl_env.
    mode;
    droot = Some dep;
    decl_tcopt = GlobalNamingOptions.get ();
  } in
  let td_tparams = List.map params (type_param env) in
  let td_type = Decl_hint.hint env concrete_type in
  let td_constraint = Option.map tcstr (Decl_hint.hint env) in
  let td_decl_errors = None in
  {
    td_vis; td_tparams; td_constraint; td_type; td_pos; td_decl_errors;
  }

and type_typedef_naming_and_decl tdef =
  let tdef = Errors.ignore_ (fun () -> Naming.typedef tdef) in
  let errors, tdecl = Errors.do_ (fun () -> typedef_decl tdef) in
  record_typedef (snd tdef.t_name);
  Decl_heap.Typedefs.add (snd tdef.t_name)
    { tdecl with td_decl_errors = Some errors};
  ()

(*****************************************************************************)
(* Global constants *)
(*****************************************************************************)

let const_decl cst =
  let open Option.Monad_infix in
  let cst_pos, _cst_name = cst.cst_name in
  let dep = Dep.GConst (snd cst.cst_name) in
  let env = {Decl_env.
    mode = cst.cst_mode;
    droot = Some dep;
    decl_tcopt = GlobalNamingOptions.get ();
  } in
  match cst.cst_type with
  | Some h -> Decl_hint.hint env h
  | None ->
    match cst.cst_value >>= Decl_utils.infer_const with
    | Some ty -> ty
    | None when FileInfo.is_strict cst.cst_mode ->
      Errors.missing_typehint cst_pos;
      Reason.Rwitness cst_pos, Tany
    | None ->
      Reason.Rwitness cst_pos, Tany

let iconst_decl cst =
  let cst = Errors.ignore_ (fun () -> Naming.global_const cst) in
  let errors, hint_ty = Errors.do_ (fun() -> const_decl cst) in
  record_const (snd cst.cst_name);
  Decl_heap.GConsts.add (snd cst.cst_name) (hint_ty, errors);
  ()

(*****************************************************************************)

let rec name_and_declare_types_program prog =
  List.iter prog begin fun def ->
    match def with
    | Ast.Namespace (_, prog) -> name_and_declare_types_program prog
    | Ast.NamespaceUse _ -> ()
    | Ast.SetNamespaceEnv _ -> ()
    | Ast.FileAttributes _ -> ()
    | Ast.Fun f -> ifun_decl f
    | Ast.Class c ->
      let class_env = { stack = SSet.empty; } in
      class_decl_if_missing class_env c
    | Ast.Typedef typedef ->
      type_typedef_decl_if_missing typedef
    | Ast.Stmt _ -> ()
    | Ast.Constant cst -> iconst_decl cst
  end

let make_env fn =
  let ast = Parser_heap.get_from_parser_heap fn in
  name_and_declare_types_program ast

let err_not_found file name =
  let err_str =
    Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file) in
raise (Decl_not_found err_str)

let declare_class_in_file file name =
  match Parser_heap.find_class_in_file file name with
  | Some cls ->
    let class_env = { stack = SSet.empty; } in
    class_decl_if_missing class_env cls
  | None ->
    err_not_found file name

let declare_fun_in_file file name =
  match Parser_heap.find_fun_in_file file name with
  | Some f -> ifun_decl f
  | None ->
    err_not_found file name

let declare_typedef_in_file file name =
  match Parser_heap.find_typedef_in_file file name with
  | Some t -> type_typedef_naming_and_decl t
  | None ->
    err_not_found file name

let declare_const_in_file file name =
  match Parser_heap.find_const_in_file file name with
  | Some cst -> iconst_decl cst
  | None ->
    err_not_found file name
