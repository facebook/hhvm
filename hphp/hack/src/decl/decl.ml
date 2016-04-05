(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Module used to declare the types.
 * For each class we want to build a complete type, that is the type of
 * the methods defined in the class plus everything that was inherited.
 *)
(*****************************************************************************)
open Core
open Nast
open Typing_defs
open Typing_deps

module DynamicYield = Decl_dynamic_yield
module Reason = Typing_reason
module Inst = Decl_instantiate
module Attrs = Attributes

module SN = Naming_special_names

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

(**
 * Adds the traits/classes which are part of a class' hierarchy.
 *
 * Traits are included in the parent list so that the class can access the trait
 * members which are declared as private/protected.
 *)
let add_grand_parents_or_traits parent_pos class_nast acc parent_type =
  let extends, is_complete, is_trait = acc in
  let class_pos = fst class_nast.c_name in
  let class_kind = class_nast.c_kind in
  if not is_trait
  then check_extend_kind parent_pos parent_type.tc_kind class_pos class_kind;
  let extends = SSet.union extends parent_type.tc_extends in
  extends, parent_type.tc_members_fully_known && is_complete, is_trait

let get_class_parent_or_trait env class_nast (parents, is_complete, is_trait)
    hint =
  let parent_pos, parent, _ = Decl_utils.unwrap_class_hint hint in
  let parents = SSet.add parent parents in
  let parent_type = Decl_env.get_class_dep env parent in
  match parent_type with
  | None ->
      (* The class lives in PHP *)
      parents, false, is_trait
  | Some parent_type ->
      (* The parent class lives in Hack *)
      let acc = parents, is_complete, is_trait in
      add_grand_parents_or_traits parent_pos class_nast acc parent_type

let get_class_parents_and_traits env class_nast =
  let parents = SSet.empty in
  let is_complete = true in
  (* extends parents *)
  let acc = parents, is_complete, false in
  let parents, is_complete, _ =
    List.fold_left class_nast.c_extends
      ~f:(get_class_parent_or_trait env class_nast) ~init:acc in
  (* traits *)
  let acc = parents, is_complete, true in
  let parents, is_complete, _ =
    List.fold_left class_nast.c_uses
      ~f:(get_class_parent_or_trait env class_nast) ~init:acc in
  (* XHP classes whose attributes were imported via "attribute :foo;" syntax *)
  let acc = parents, is_complete, true in
  let parents, is_complete, _ =
    List.fold_left class_nast.c_xhp_attr_uses
      ~f:(get_class_parent_or_trait env class_nast) ~init:acc in
  parents, is_complete

(*****************************************************************************)
(* Section declaring the type of a function *)
(*****************************************************************************)

let rec ifun_decl tcopt (f: Ast.fun_) =
  let f = Naming.fun_ tcopt f in
  let cid = snd f.f_name in
  Naming_heap.FunHeap.add cid f;
  fun_decl f;
  ()

and make_param_ty env param =
  let param_pos = (fst param.param_id) in
  let ty = match param.param_hint with
    | None ->
      let r = Reason.Rwitness param_pos in
      (r, Tany)
      (* if the code is strict, use the type-hint *)
    | Some x ->
      Decl_hint.hint env x
  in
  let ty = match ty with
    | _, t when param.param_is_variadic ->
      (* When checking a call f($a, $b) to a function f(C ...$args),
       * both $a and $b must be of type C *)
      Reason.Rvar_param param_pos, t
    | x -> x
  in
  Some param.param_name, ty

and fun_decl f =
  let dep = Dep.Fun (snd f.f_name) in
  let env = {Decl_env.mode = f.f_mode; droot = Some dep} in
  let ft = fun_decl_in_env env f in
  Typing_heap.Funs.add (snd f.f_name) ft;
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
    | Ast.FSync -> ty_any

and fun_decl_in_env env f =
  let arity_min, params = make_params env f.f_params in
  let ret_ty = match f.f_ret with
    | None -> ret_from_fun_kind (fst f.f_name) f.f_fun_kind
    | Some ty -> Decl_hint.hint env ty in
  let arity = match f.f_variadic with
    | FVvariadicArg param ->
      assert param.param_is_variadic;
      assert (param.param_expr = None);
      let p_name, p_ty = make_param_ty env param in
      Fvariadic (arity_min, (p_name, p_ty))
    | FVellipsis    -> Fellipsis (arity_min)
    | FVnonVariadic -> Fstandard (arity_min, List.length f.f_params)
  in
  let tparams = List.map f.f_tparams (type_param env) in
  let ft = {
    ft_pos         = fst f.f_name;
    ft_deprecated  =
      Attributes.deprecated ~kind:"function" f.f_name f.f_user_attributes;
    ft_abstract    = false;
    ft_arity       = arity;
    ft_tparams     = tparams;
    ft_params      = params;
    ft_ret         = ret_ty;
  } in
  ft

and type_param env (variance, x, cstr) =
  let cstr = match cstr with
    | Some (ck, h) ->
        let ty = Decl_hint.hint env h in
        Some (ck, ty)
    | None -> None in
  variance, x, cstr

and check_default pos mandatory e =
  if not mandatory && e = None
  then Errors.previous_default pos
  else ()

(* Functions building the types for the parameters of a function *)
(* It's not completely trivial because of optional arguments  *)
and make_param env mandatory arity param =
  let ty = make_param_ty env param in
  let mandatory =
    if param.param_is_variadic then begin
      assert(param.param_expr = None);
      false
    end else begin
      check_default (fst param.param_id) mandatory param.param_expr;
      mandatory && param.param_expr = None
    end
  in
  let arity = if mandatory then arity + 1 else arity in
  arity, mandatory, ty

and make_params env paraml =
  let rec loop mandatory arity paraml =
    match paraml with
    | [] -> arity, []
    | param :: rl ->
        let arity, mandatory, ty = make_param env mandatory arity param in
        let arity, rest = loop mandatory arity rl in
        arity, ty :: rest
  in
  loop true 0 paraml

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
    if Naming_heap.ClassHeap.mem cid then () else
      class_naming_and_decl class_env cid c
  end

and class_naming_and_decl (class_env:class_env) cid c =
  let class_env = { class_env with stack = SSet.add cid class_env.stack } in
  let c = Naming.class_ class_env.tcopt c in
  class_parents_decl class_env c;
  class_decl class_env.tcopt c;
  (* It is important to add the "named" ast (nast.ml) only
   * AFTER we are done declaring the type type of the class.
   * Otherwise there is a subtle race condition.
   *
   * Worker 1: looks up class A. Sees that class A needs
   * to be recomputed, starts to recompute the type of A
   *
   * Worker 2: loops up class A, sees that the named Ast for
   * A is there, deduces that the type has already been computed
   * and could end up using an old version of the class type if
   * Worker 1 didn't finish.
   *
   * This race doesn't occur if we set the named Ast for class A
   * AFTER we are done declaring the type of A.
   * The worst case scenario is both workers recompute the same type
   * which is OK.
   *)
  Naming_heap.ClassHeap.add cid c;
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

and class_hint_decl class_env hint =
  match hint with
  | _, Happly ((_, cid), _) ->
    begin match Naming_heap.TypeIdHeap.get cid with
      | Some (p, `Class) when not (Naming_heap.ClassHeap.mem cid) ->
        (* We are supposed to redeclare the class *)
        let fn = Pos.filename p in
        let class_opt = Parser_heap.find_class_in_file fn cid in
        Option.iter class_opt (class_decl_if_missing class_env)
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
  let _p, cls_name = c.c_name in
  let class_dep = Dep.Class cls_name in
  let env = {Decl_env.mode = c.c_mode; droot = Some class_dep} in
  let inherited = Decl_inherit.make env c in
  let props = inherited.Decl_inherit.ih_props in
  let props =
    List.fold_left ~f:(class_var_decl env c) ~init:props c.c_vars in
  let m = inherited.Decl_inherit.ih_methods in
  let m =
    List.fold_left ~f:(method_decl_acc env c) ~init:m c.c_methods in
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
  let sm = List.fold_left c.c_static_methods
    ~f:(method_decl_acc env c) ~init:sm in
  SMap.iter (check_static_method m) sm;
  let parent_cstr = inherited.Decl_inherit.ih_cstr in
  let cstr = constructor_decl env parent_cstr c in
  let has_concrete_cstr = match (fst cstr) with
    | None
    | Some {ce_type = (_, Tfun ({ft_abstract = true; _})); _} -> false
    | _ -> true in
  let impl = c.c_extends @ c.c_implements @ c.c_uses in
  let impl = List.map impl (Decl_hint.hint env) in
  let impl = match SMap.get SN.Members.__toString m with
    | Some {ce_type = (_, Tfun ft); _} when cls_name <> SN.Classes.cStringish ->
      (* HHVM implicitly adds Stringish interface for every class/iface/trait
       * with a __toString method; "string" also implements this interface *)
      let pos = ft.ft_pos in
      let ty = (Reason.Rhint pos, Tapply ((pos, SN.Classes.cStringish), [])) in
      ty :: impl
    | _ -> impl
  in
  let impl = List.map impl (get_implements env) in
  let impl = List.fold_right impl ~f:(SMap.fold SMap.add) ~init:SMap.empty in
  let extends, ext_strict = get_class_parents_and_traits env c in
  let extends = if c.c_is_xhp
    then SSet.add "XHP" extends
    else extends
  in
  let req_ancestors, req_ancestors_extends =
    Decl_requirements.get_class_requirements env c in
  let m = if DynamicYield.is_dynamic_yield (snd c.c_name)
    then DynamicYield.clean_dynamic_yield m
    else m in
  let dy_check = match c.c_kind with
    | Ast.Cenum -> false
    | Ast.Cabstract
    | Ast.Cnormal -> DynamicYield.contains_dynamic_yield extends
    | Ast.Cinterface
    | Ast.Ctrait ->
      (* NOTE: Only the DynamicYield trait should provide
       * IUseDynamicYield via implementation; all other traits should
       * use 'require implements IUseDynamicYield' *)
      (DynamicYield.implements_dynamic_yield_interface impl
       || DynamicYield.contains_dynamic_yield_interface req_ancestors_extends
       || DynamicYield.contains_dynamic_yield req_ancestors_extends)
  in
  let m = if dy_check then DynamicYield.decl m else m in
  let ext_strict = List.fold_left c.c_uses
    ~f:(trait_exists env) ~init:ext_strict in
  let unsafe_xhp = TypecheckerOptions.unsafe_xhp tcopt in
  let not_strict_because_xhp = unsafe_xhp && c.c_is_xhp in
  if not ext_strict && not not_strict_because_xhp &&
      (env.Decl_env.mode = FileInfo.Mstrict) then
    let p, name = c.c_name in
    Errors.strict_members_not_known p name
  else ();
  let ext_strict = if not_strict_because_xhp then false else ext_strict in
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
  let tc = {
    tc_final = c.c_final;
    tc_abstract = is_abstract;
    tc_need_init = has_concrete_cstr;
    tc_deferred_init_members = deferred_members;
    tc_members_fully_known = ext_strict;
    tc_kind = c.c_kind;
    tc_name = snd c.c_name;
    tc_pos = fst c.c_name;
    tc_tparams = tparams;
    tc_consts = consts;
    tc_typeconsts = typeconsts;
    tc_props = props;
    tc_sprops = sprops;
    tc_methods = m;
    tc_smethods = sm;
    tc_construct = cstr;
    tc_ancestors = impl;
    tc_extends = extends;
    tc_req_ancestors = req_ancestors;
    tc_req_ancestors_extends = req_ancestors_extends;
    tc_enum_type = enum;
  } in
  if Ast.Cnormal = c.c_kind then
    begin
      SMap.iter (method_check_trait_overrides c) m;
      SMap.iter (method_check_trait_overrides c) sm;
    end
  else ();
  SMap.iter begin fun x _ ->
    Typing_deps.add_idep class_dep (Dep.Class x)
  end impl;
  Typing_heap.Classes.add (snd c.c_name) tc

and get_implements env ht =
  let _r, (_p, c), paraml = Decl_utils.unwrap_class_type ht in
  let class_ = Decl_env.get_class_dep env c in
  match class_ with
  | None ->
      (* The class lives in PHP land *)
      SMap.singleton c ht
  | Some class_ ->
      let subst = Inst.make_subst class_.tc_tparams paraml in
      let sub_implements =
        SMap.map
          (fun ty -> Inst.instantiate subst ty)
          class_.tc_ancestors
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

and check_static_method obj method_name { ce_type = (reason_for_type, _); _ } =
  if SMap.mem method_name obj
  then begin
    let static_position = Reason.to_pos reason_for_type in
    let dyn_method = SMap.find_unsafe method_name obj in
    let dyn_position = Reason.to_pos (fst dyn_method.ce_type) in
    Errors.static_dynamic static_position dyn_position method_name
  end
  else ()

and constructor_decl env (pcstr, pconsist) class_ =
  (* constructors in children of class_ must be consistent? *)
  let cconsist = class_.c_final ||
    Attrs.mem
      SN.UserAttributes.uaConsistentConstruct
      class_.c_user_attributes in
  match class_.c_constructor, pcstr with
  | None, _ -> pcstr, cconsist || pconsist
  | Some method_, Some {ce_final = true; ce_type = (r, _); _ } ->
    Errors.override_final ~parent:(Reason.to_pos r) ~child:(fst method_.m_name);
    let cstr, mconsist = build_constructor env class_ method_ in
    cstr, cconsist || mconsist || pconsist
  | Some method_, _ ->
    let cstr, mconsist = build_constructor env class_ method_ in
    cstr, cconsist || mconsist || pconsist

and build_constructor env class_ method_ =
  let ty = method_decl env method_ in
  let _, class_name = class_.c_name in
  let vis = visibility class_name method_.m_visibility in
  let mconsist = method_.m_final || class_.c_kind == Ast.Cinterface in
  (* due to the requirement of calling parent::__construct, a private
   * constructor cannot be overridden *)
  let mconsist = mconsist || method_.m_visibility == Private in
  let mconsist = match ty with
    | (_, Tfun ({ft_abstract = true; _})) -> true
    | _ -> mconsist in
  (* the alternative to overriding
   * UserAttributes.uaConsistentConstruct is marking the corresponding
   * 'new static()' UNSAFE, potentially impacting the safety of a large
   * type hierarchy. *)
  let consist_override =
    Attrs.mem SN.UserAttributes.uaUnsafeConstruct method_.m_user_attributes in
  let cstr = {
    ce_final = method_.m_final;
    ce_is_xhp_attr = false;
    ce_override = consist_override;
    ce_synthesized = false;
    ce_visibility = vis;
    ce_type = ty;
    ce_origin = class_name;
  } in
  Some cstr, mconsist

and class_const_decl env c acc (h, id, e) =
  let c_name = (snd c.c_name) in
  let ty =
    match h, e with
      | Some h, Some _ -> Decl_hint.hint env h
      | Some h, None ->
        let h_ty = Decl_hint.hint env h in
        let pos, name = id in
        Reason.Rwitness pos,
          Tgeneric (c_name^"::"^name, Some (Ast.Constraint_as, h_ty))
      | None, Some e -> begin
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
            if c.c_mode = FileInfo.Mstrict && c.c_kind <> Ast.Cenum
            then Errors.missing_typehint (fst id);
            Reason.Rwitness (fst id), Tany
        in
        infer_const e
      end
      | None, None ->
        let pos, name = id in
        if c.c_mode = FileInfo.Mstrict then Errors.missing_typehint pos;
        let r = Reason.Rwitness pos in
        let const_ty = r, Tgeneric (c_name^"::"^name,
          Some (Ast.Constraint_as, (r, Tany))) in
        const_ty
  in
  let cc = {
    cc_synthesized = false;
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
  let ce = {
    ce_final = true; ce_is_xhp_attr = cv.cv_is_xhp; ce_override = false;
    ce_synthesized = false; ce_visibility = vis; ce_type = ty;
    ce_origin = (snd c.c_name);
  } in
  let acc = SMap.add id ce acc in
  acc

and static_class_var_decl env c acc cv =
  let ty = match cv.cv_type with
    | None -> Reason.Rwitness (fst cv.cv_id), Tany
    | Some ty -> Decl_hint.hint env ty in
  let id = snd cv.cv_id in
  let vis = visibility (snd c.c_name) cv.cv_visibility in
  let ce = { ce_final = true; ce_is_xhp_attr = cv.cv_is_xhp; ce_override = false;
             ce_synthesized = false; ce_visibility = vis; ce_type = ty;
             ce_origin = (snd c.c_name);
           } in
  let acc = SMap.add ("$"^id) ce acc in
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
and typeconst_ty_decl pos c_name tc_name ~is_abstract =
  let r = Reason.Rwitness pos in
  let tsid = pos, SN.FB.cTypeStructure in
  let ts_ty = r, Tapply (tsid, [r, Taccess ((r, Tthis), [pos, tc_name])]) in
  let ts_ty = if is_abstract
    then r, Tgeneric (c_name^"::"^tc_name, Some (Ast.Constraint_as, ts_ty))
    else ts_ty in
  {
    cc_synthesized = true;
    cc_type        = ts_ty;
    cc_expr        = None;
    cc_origin      = tc_name;
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
      let ts = typeconst_ty_decl pos (snd c.c_name) name ~is_abstract in
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
  let arity_min, params = make_params env m.m_params in
  let ret = match m.m_ret with
    | None -> ret_from_fun_kind (fst m.m_name) m.m_fun_kind
    | Some ret -> Decl_hint.hint env ret in
  let arity = match m.m_variadic with
    | FVvariadicArg param ->
      assert param.param_is_variadic;
      assert (param.param_expr = None);
      let p_name, p_ty = make_param_ty env param in
      Fvariadic (arity_min, (p_name, p_ty))
    | FVellipsis    -> Fellipsis arity_min
    | FVnonVariadic -> Fstandard (arity_min, List.length m.m_params)
  in
  let tparams = List.map m.m_tparams (type_param env) in
  let ft = {
    ft_pos      = fst m.m_name;
    ft_deprecated =
      Attrs.deprecated ~kind:"method" m.m_name m.m_user_attributes;
    ft_abstract = m.m_abstract;
    ft_arity    = arity;
    ft_tparams  = tparams;
    ft_params   = params;
    ft_ret      = ret;
  } in
  let ty = Reason.Rwitness (fst m.m_name), Tfun ft in
  ty

and method_check_override c m acc =
  let pos, id = m.m_name in
  let _, class_id = c.c_name in
  let override = Attrs.mem SN.UserAttributes.uaOverride m.m_user_attributes in
  if m.m_visibility = Private && override then
    Errors.private_override pos class_id id;
  match SMap.get id acc with
    | Some { ce_final = is_final; ce_type = (r, _); _ } ->
      if is_final then
        Errors.override_final ~parent:(Reason.to_pos r) ~child:pos;
      false
    | None when override && c.c_kind = Ast.Ctrait -> true
    | None when override ->
      Errors.should_be_override pos class_id id;
      false
    | None -> false

and method_decl_acc env c acc m =
  let check_override = method_check_override c m acc in
  let ty = method_decl env m in
  let _, id = m.m_name in
  let vis =
    match SMap.get id acc, m.m_visibility with
      | Some { ce_visibility = Vprotected _ as parent_vis; _ }, Protected ->
        parent_vis
      | _ -> visibility (snd c.c_name) m.m_visibility
  in
  let ce = {
    ce_final = m.m_final; ce_is_xhp_attr = false; ce_override = check_override;
    ce_synthesized = false; ce_visibility = vis; ce_type = ty;
    ce_origin = snd (c.c_name);
  } in
  let acc = SMap.add id ce acc in
  acc

and method_check_trait_overrides c id method_ce =
  if method_ce.ce_override then
    Errors.override_per_trait
      c.c_name id (Reason.to_pos (fst method_ce.ce_type))

(*****************************************************************************)
(* Dealing with typedefs *)
(*****************************************************************************)

let rec type_typedef_decl_if_missing tcopt typedef =
  let _, tid = typedef.Ast.t_id in
  if Naming_heap.TypedefHeap.mem tid
  then ()
  else
    type_typedef_naming_and_decl tcopt typedef

and type_typedef_naming_and_decl tcopt tdef =
  let {
    t_pos = _;
    t_tparams = params;
    t_constraint = tcstr;
    t_kind = concrete_type;
    t_user_attributes = _;
  } as decl = Naming.typedef tcopt tdef in
  let td_pos, tid = tdef.Ast.t_id in
  let dep = Typing_deps.Dep.Class tid in
  let env = {Decl_env.mode = tdef.Ast.t_mode; droot = Some dep} in
  let td_tparams = List.map params (type_param env) in
  let td_type = Decl_hint.hint env concrete_type in
  let td_constraint = Option.map tcstr (Decl_hint.hint env) in
  let td_vis = match tdef.Ast.t_kind with
    | Ast.Alias _ -> Transparent
    | Ast.NewType _ -> Opaque in
  let tdecl = {
    td_vis; td_tparams; td_constraint; td_type; td_pos;
  } in
  Typing_heap.Typedefs.add tid tdecl;
  Naming_heap.TypedefHeap.add tid decl;
  ()

(*****************************************************************************)
(* Global constants *)
(*****************************************************************************)

let iconst_decl tcopt cst =
  let cst = Naming.global_const tcopt cst in
  let _cst_pos, cst_name = cst.cst_name in
  Naming_heap.ConstHeap.add cst_name cst;
  let dep = Dep.GConst (snd cst.cst_name) in
  let env = {Decl_env.mode = cst.cst_mode; droot = Some dep} in
  let hint_ty =
    match cst.cst_type with
    | None -> Reason.Rnone, Tany
    | Some h -> Decl_hint.hint env h
  in
  Typing_heap.GConsts.add cst_name hint_ty;
  ()

(*****************************************************************************)

let name_and_declare_types_program tcopt prog =
  List.iter prog begin fun def ->
    match def with
    | Ast.Namespace _
    | Ast.NamespaceUse _ -> assert false
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
  match Parser_heap.ParserHeap.get fn with
  | None -> ()
  | Some prog ->
    name_and_declare_types_program tcopt prog

module Error_list = Shared_list.Make(struct
  type element = Errors.error
  let max_size = 10 * 1024 * 1024 * 1024
end)

module Failed_list = Shared_list.Make(struct
  type element = Relative_path.t
  let max_size = 10 * 1024 * 1024 * 1024
end)

(* Returns a list of files that are considered to have failed decl and must
 * be redeclared every time the typechecker discovers a file change *)
let failures_from_errors decl_errors fn =
  List.fold_left decl_errors ~f:begin fun failed error ->
    (* It is important to add the file that is the cause of the failure.
     * What can happen is that during a declaration phase, we realize
     * that a parent class is outdated. When this happens, we redeclare
     * the class, even if it is in a different file. Therefore, the file
     * where the error occurs might be different from the file we
     * are declaring right now.
     *)
    let file_with_error = Pos.filename (Errors.get_pos error) in
    assert (file_with_error <> Relative_path.default);
    let failed = Relative_path.Set.add failed file_with_error in
    let failed = Relative_path.Set.add failed fn in
    failed
  end ~init:Relative_path.Set.empty

let run_and_record_errors file f =
  let errors, () = Errors.do_ f in
  let failures = failures_from_errors errors file in
  Relative_path.Set.iter failures Failed_list.append;
  List.iter errors Error_list.append;
  ()

let declare_class_in_file tcopt file name =
  match Parser_heap.find_class_in_file file name with
  | Some cls ->
    let class_env = { tcopt; stack = SSet.empty; } in
    run_and_record_errors file begin fun () ->
      class_decl_if_missing class_env cls
    end;
  | None -> raise Not_found

let declare_fun_in_file tcopt file name =
  match Parser_heap.find_fun_in_file file name with
  | Some f ->
    run_and_record_errors file begin fun () ->
      ifun_decl tcopt f
    end
  | None -> raise Not_found

let declare_typedef_in_file tcopt file name =
  match Parser_heap.find_typedef_in_file file name with
  | Some t ->
    run_and_record_errors file begin fun () ->
      type_typedef_naming_and_decl tcopt t
    end
  | None -> raise Not_found

let declare_const_in_file tcopt file name =
  match Parser_heap.find_const_in_file file name with
  | Some cst ->
    run_and_record_errors file begin fun () ->
      iconst_decl tcopt cst
    end
  | None -> raise Not_found

let errors_and_failures () =
  Error_list.get (), Failed_list.get ()

let reset_errors () =
  Error_list.reset ();
  Failed_list.reset ()
