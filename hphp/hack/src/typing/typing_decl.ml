(**
 * Copyright (c) 2014, Facebook, Inc.
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
open Utils
open Typing_defs
open Nast
open Typing_deps

module Env = Typing_env
module DynamicYield = Typing_dynamic_yield

(*****************************************************************************)
(* Module used to track what classes are declared and which ones still need
 * to be processed. The declaration phase happens in parallel. Because of that
 * a worker needs to know if the type of a class is ready or if it needs to
 * be recomputed. When ClassStatus.mem *class-name* returns true, we know
 * the class has already been recomputed by a different worker.
 * TypedefStatus is a similar mechanism for typedefs.
 *)
(*****************************************************************************)

module ClassStatus = SharedMem.NoCache(struct
  type t = unit
  let prefix = Prefix.make()
end)

let is_class_ready class_name =
  ClassStatus.mem class_name

let report_class_ready class_name =
  ClassStatus.add class_name ()

let remove_classes class_set =
  ClassStatus.remove_batch class_set

(*****************************************************************************)
(* Checking that the kind of a class is compatible with its parent
 * For example, a class cannot extend an interface, an interface cannot
 * extend a trait etc ...
 *)
(*****************************************************************************)

let check_extend_kind env parent_pos parent_kind child_pos child_kind c =
  match parent_kind, child_kind with
    (* What is allowed *)
  | (Ast.Cabstract | Ast.Cnormal), (Ast.Cabstract | Ast.Cnormal)
  | Ast.Ctrait, Ast.Ctrait
  | Ast.Cinterface, Ast.Cinterface ->
      ()
  | _ when not (Env.is_decl env) (* Don't check in decl mode *) ->
      (* What is dissallowed *)
      let parent = Ast.string_of_class_kind parent_kind in
      let child  = Ast.string_of_class_kind child_kind in
      let msg1 = child_pos, child^" cannot extend "^parent in
      let msg2 = parent_pos, "This is "^parent in
      Errors.add_list [msg1; msg2]
  | _ -> ()


(*****************************************************************************)
(* Functions used retrieve everything implemented in parent classes
 * The return values:
 * env: the new environment
 * parents: the name of all the parents and grand parents of the class this
 *          includes traits.
 * is_complete: true if all the parents live in Hack
 *)
(*****************************************************************************)

let unpack_hint = function
  | (_, Happly ((parent_pos, parent_name), _)) ->
      parent_pos, parent_name
  | _ -> assert false

(**
 * Adds the traits/classes which are part of a class' hierarchy.
 *
 * Traits are included in the parent list so that the class can access the trait
 * members which are declared as private/protected.
 *)
let add_grand_parents_or_traits parent_pos class_nast acc parent_type =
  let env, extends, is_complete, is_trait = acc in
  let class_pos = fst class_nast.c_name in
  let class_kind = class_nast.c_kind in
  if not is_trait
  then check_extend_kind env parent_pos parent_type.tc_kind class_pos class_kind class_nast;
  let extends = SSet.union extends parent_type.tc_extends in
  env, extends, parent_type.tc_members_fully_known && is_complete, is_trait

let get_class_parent_or_trait class_nast (env, parents, is_complete, is_trait) hint =
  let parent_pos, parent = unpack_hint hint in
  let parents = SSet.add parent parents in
  let env, parent_type = Env.get_class_dep env parent in
  match parent_type with
  | None ->
      (* The class lives in PHP *)
      env, parents, false, is_trait
  | Some parent_type ->
      (* The parent class lives in Hack *)
      let acc = env, parents, is_complete, is_trait in
      add_grand_parents_or_traits parent_pos class_nast acc parent_type

let get_class_parents_and_traits env class_nast =
  let parents = SSet.empty in
  let is_complete = true in
  (* extends parents *)
  let acc = env, parents, is_complete, false in
  let env, parents, is_complete, _ =
    List.fold_left (get_class_parent_or_trait class_nast) acc class_nast.c_extends in
  (* traits *)
  let acc = env, parents, is_complete, true in
  let env, parents, is_complete, _ =
    List.fold_left (get_class_parent_or_trait class_nast) acc class_nast.c_uses in
  env, parents, is_complete

let error_trait_req pos req =
  Errors.add pos ("Failure to satisfy trait requirement: "^(Utils.strip_ns req))

(* for non-traits, check that each used trait's requirements have been
 * satisfied; for traits, accumulate the requirements so that we can
 * successfully check the bodies of trait methods *)
let merge_parent_trait_reqs class_nast impls
    (env, req_ancestors, req_ancestors_extends) trait_hint =
  let parent_pos, parent = unpack_hint trait_hint in
  let env, parent_type = Env.get_class_dep env parent in
  match parent_type with
    | None ->
      (* The class lives in PHP *)
      env, req_ancestors, req_ancestors_extends
    | Some parent_type when (class_nast.c_kind != Ast.Ctrait) ->
      SSet.iter begin fun req ->
        if SMap.mem req impls then () (* requirement satisfied *)
        else error_trait_req parent_pos req
      end parent_type.tc_req_ancestors;
      env, req_ancestors, req_ancestors_extends
    | Some parent_type ->
      let req_ancestors = SSet.union parent_type.tc_req_ancestors req_ancestors in
      let req_ancestors_extends =
        SSet.union parent_type.tc_req_ancestors_extends req_ancestors_extends in
      env, req_ancestors, req_ancestors_extends

let get_trait_req class_nast impls (env, requirements, req_extends) hint =
  let parent_pos, req = unpack_hint hint in
  if class_nast.c_kind != Ast.Ctrait && not (SMap.mem req impls) then
    error_trait_req parent_pos req;
  let requirements = SSet.add req requirements in
  let req_extends = SSet.add req req_extends in
  let env, req_type = Env.get_class_dep env req in
  match req_type with
  | None ->
      (* The class lives in PHP : error?? *)
      env, requirements, req_extends
  | Some parent_type ->
      (* The parent class lives in Hack *)
      env, requirements, SSet.union req_extends parent_type.tc_extends

let get_trait_requirements env class_nast impls =
  let req_ancestors = SSet.empty in
  let req_ancestors_extends = SSet.empty in
  let acc = (env, req_ancestors, req_ancestors_extends) in
  let acc =
    List.fold_left (get_trait_req class_nast impls)
      acc class_nast.c_req_extends in
  let acc =
    List.fold_left (get_trait_req class_nast impls)
      acc class_nast.c_req_implements in
  let acc =
    List.fold_left (merge_parent_trait_reqs class_nast impls)
      acc class_nast.c_uses in
  if class_nast.c_kind != Ast.Ctrait then
    (* for a non-trait, requirements have been checked ... nothing to save *)
    env, SSet.empty, SSet.empty
  else
    (* for a trait, return the accumulated list of direct and
     * inherited requirements *)
    acc

(*****************************************************************************)
(* Section declaring the type of a function *)
(*****************************************************************************)

let ifun_decl nenv (f: Ast.fun_) =
  let f = Naming.fun_ nenv f in
  let cid = snd f.f_name in
  Naming_heap.FunHeap.add cid f;
  Typing.fun_decl f;
  ()

(*****************************************************************************)
(* Section declaring the type of a class *)
(*****************************************************************************)

type class_env = {
    nenv: Naming.env;
    stack: SSet.t;
    all_classes: SSet.t SMap.t;
  }

let error_cyclic stack pos =
  let stack = SSet.fold (fun x y -> (Utils.strip_ns x)^" "^y) stack "" in
  Errors.add pos ("Cyclic class definition : "^stack)

let error_final ~parent ~child =
  Errors.add_list [child, "You cannot override this method";
                   parent, "It was declared as final"]

let check_if_cyclic class_env (pos, cid) =
  let stack = class_env.stack in
  let is_cyclic = SSet.mem cid stack in
  if is_cyclic
  then error_cyclic stack pos;
  is_cyclic

let rec class_decl_if_missing_opt class_env = function
  | None -> ()
  | Some c -> class_decl_if_missing class_env c

and class_decl_if_missing class_env c =
  let pos, cid as c_name = c.Ast.c_name in
  if check_if_cyclic class_env c_name
  then ()
  else begin
    if Naming_heap.ClassHeap.mem cid then () else
    class_naming_and_decl class_env cid c
  end

and class_naming_and_decl class_env cid c =
  let class_env = { class_env with stack = SSet.add cid class_env.stack } in
  let c = Naming.class_ class_env.nenv c in
  class_parents_decl class_env c;
  class_decl c;
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
  report_class_ready cid

and class_parents_decl class_env c =
  let class_hint = class_hint_decl class_env in
  List.iter class_hint c.c_extends;
  List.iter class_hint c.c_implements;
  List.iter class_hint c.c_uses;
  List.iter class_hint c.c_req_extends;
  List.iter class_hint c.c_req_implements;
  ()

and class_hint_decl class_env hint =
  match hint with
    | _, Happly ((p, cid), _)
      when SMap.mem cid class_env.all_classes && not (is_class_ready cid) ->
      (* We are supposed to redeclare the class *)
        let files = SMap.find_unsafe cid class_env.all_classes in
        SSet.iter begin fun fn ->
          let class_opt = Parser_heap.find_class_in_file fn cid in
          class_decl_if_missing_opt class_env class_opt
        end files
    | _ ->
      (* This class lives in PHP land *)
      ()

and class_is_abstract c =
  match c.c_kind with
    | Ast.Cabstract | Ast.Cinterface | Ast.Ctrait -> true
    | _ -> false

and class_decl c =
  let is_abstract = class_is_abstract c in
  let cls_pos, cls_name = c.c_name in
  let env = Typing_env.empty (Pos.filename cls_pos) in
  let env = Env.set_mode env c.c_mode in
  let class_dep = Dep.Class cls_name in
  let env = Env.set_root env class_dep in
  let env, inherited = Typing_inherit.make env c in
  let cvars = inherited.Typing_inherit.ih_cvars in
  let env, cvars = List.fold_left (class_var_decl c) (env, cvars) c.c_vars in
  let m = inherited.Typing_inherit.ih_methods in
  let env, m = List.fold_left (method_decl_acc c) (env, m) c.c_methods in
  let consts = inherited.Typing_inherit.ih_consts in
  let env, consts =
    List.fold_left (class_const_decl c) (env, consts) c.c_consts in
  let consts = SMap.add "class" (class_class_decl c.c_name) consts in
  let sclass_var = static_class_var_decl c in
  let scvars = inherited.Typing_inherit.ih_scvars in
  let env, scvars = List.fold_left sclass_var (env, scvars) c.c_static_vars in
  let sm = inherited.Typing_inherit.ih_smethods in
  let env, sm = List.fold_left (method_decl_acc c) (env, sm) c.c_static_methods in
  SMap.iter (check_static_method m) sm;
  let parent_cstr = inherited.Typing_inherit.ih_cstr in
  let env, cstr = constructor_decl env parent_cstr c in
  let need_init = match cstr with
    | None
    | Some {ce_type = (_, Tfun ({ft_abstract = true; _})); _} -> false
    | _ -> true in
  let impl = c.c_extends @ c.c_implements @ c.c_uses in
  let impl = match SMap.get "__toString" m with
    | Some {ce_type = (_, Tfun ft); _} when cls_name <> "\\Stringish" ->
      (* HHVM implicitly adds Stringish interface for every class/iface/trait
       * with a __toString method; "string" also implements this interface *)
      let pos = ft.ft_pos in
      let h = (pos, Nast.Happly ((pos, "\\Stringish"), [])) in
      h :: impl
    | _ -> impl
  in
  let self = Typing.get_self_from_c env c in
  let env, impl_dimpl =
    lfold (Typing.get_implements ~with_checks:false ~this:self) env impl in
  let impl, dimpl = List.split impl_dimpl in
  let impl = List.fold_right (SMap.fold SMap.add) impl SMap.empty in
  let dimpl = List.fold_right (SMap.fold SMap.add) dimpl SMap.empty in
  let env, extends, ext_strict = get_class_parents_and_traits env c in
  let extends = if c.c_is_xhp
    then SSet.add "XHP" extends
    else extends
  in
  let env, req_ancestors, req_ancestors_extends =
    get_trait_requirements env c impl in
  let env, m = if DynamicYield.is_dynamic_yield (snd c.c_name)
    then DynamicYield.clean_dynamic_yield env m
    else env, m in
  let dy_check = match c.c_kind with
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
  let env, m = if dy_check then DynamicYield.decl env m else env, m in
  let ext_strict = List.fold_left (trait_exists env) ext_strict c.c_uses in
  let ext_strict = not c.c_is_xhp && ext_strict in
  let self_dimpl = if is_abstract then impl else SMap.empty in
  let dimpl =
    if is_abstract
    then SMap.fold SMap.add self_dimpl dimpl
    else dimpl
  in
  let env, tparams = lfold Typing.type_param env c.c_tparams in
  let tc = {
    tc_final = c.c_final;
    tc_abstract = is_abstract;
    tc_need_init = need_init;
    tc_members_init = NastInitCheck.class_decl env c;
    tc_members_fully_known = ext_strict;
    tc_kind = c.c_kind;
    tc_name = snd c.c_name;
    tc_pos = fst c.c_name;
    tc_tparams = tparams;
    tc_consts = consts;
    tc_cvars = cvars;
    tc_scvars = scvars;
    tc_methods = m;
    tc_smethods = sm;
    tc_construct = cstr;
    tc_ancestors = impl;
    tc_ancestors_checked_when_concrete = dimpl;
    tc_extends = extends;
    tc_req_ancestors = req_ancestors;
    tc_req_ancestors_extends = req_ancestors_extends;
    tc_user_attributes = c.c_user_attributes;
  } in
  if Ast.Cnormal = c.c_kind then
    SMap.iter (method_check_trait_overrides c) m
  else ();
  if Ast.Cnormal = c.c_kind then
    SMap.iter (method_check_trait_overrides c) sm
  else ();
  SMap.iter begin fun x _ ->
    Typing_deps.add_idep (Some class_dep) (Dep.Class x)
  end impl;
  SMap.iter begin fun x _ ->
    Typing_deps.add_idep (Some class_dep) (Dep.Class x)
  end dimpl;
  Env.add_class (snd c.c_name) tc

and trait_exists env acc trait =
  match trait with
    | (_, Happly ((p2, trait), _)) ->
      let env, class_ = Env.get_class_dep env trait in
      (match class_ with
        | None -> false
        | Some class_ -> acc
      )
    | _ -> false

and check_static_method obj method_name { ce_type = (reason_for_type, _); _ } =
  if SMap.mem method_name obj
  then begin
    let static_position = Reason.to_pos reason_for_type in
    let dyn_method = SMap.find_unsafe method_name obj in
    let dyn_position = Reason.to_pos (fst dyn_method.ce_type) in
    let msg_static = "The function "^method_name^" is static" in
    let msg_dynamic = "It is defined as dynamic here" in
    Errors.add_list [static_position, msg_static; dyn_position, msg_dynamic]
  end
  else ()

and constructor_decl env pcstr class_ =
  match class_.c_constructor, pcstr with
    | None, Some cstr ->
        env, Some cstr
    | Some method_, Some { ce_final = true; ce_type = (r, _); _ } ->
      error_final ~parent:(Reason.to_pos r) ~child:(fst method_.m_name);
      build_constructor env class_ method_
    | Some method_, _ ->
      build_constructor env class_ method_
    | None, _ ->
        env, None

and build_constructor env class_ method_ =
  let env, ty = method_decl class_ env method_ in
  let _, class_name = class_.c_name in
  let vis = visibility class_name method_.m_visibility in
  let cstr = {
    ce_final = method_.m_final;
    ce_override = false ;
    ce_synthesized = false;
    ce_visibility = vis;
    ce_type = ty;
    ce_origin = class_name;
  } in
  env, Some cstr

and class_const_decl c (env, acc) (h, id, e) =
  let env, ty =
    match h with
      | None ->
        (match snd e with
          | String _
          | String2 ([], _)
          | True
          | False
          | Int _
          | Float _
          | Array _ ->
            let _, ty = Typing.expr env e in
              (* We don't want to keep the environment of the inference
               * CAREFULL, right now, array is just Tarray, with no
               * type variable, if we were to add parameters array<T>,
               * we would have to: make a full expansion, that is,
               * replace all the type variables in ty by their "true" type,
               * because this feature doesn't exist, this isn't necessary
               * right now. I am adding this tag "array", because I know
               * I would search for it if I was changing the way arrays are
               * typed.
               *)
            env, ty
          | _ ->
            env, (Reason.Rwitness (fst id), Tany)
        )
      | Some h -> Typing_hint.hint env h
  in
  let ce = { ce_final = true; ce_override = false; ce_synthesized = false;
             ce_visibility = Vpublic; ce_type = ty; ce_origin = (snd c.c_name);
           } in
  let acc = SMap.add (snd id) ce acc in
  env, acc

(* Every class, interface, and trait implicitly defines ::class to
 * allow accessing its fully qualified name as a string *)
and class_class_decl class_id =
  let pos, name = class_id in
  let reason = Reason.Rclass_class (pos, name) in
  {
    ce_final      = false;
    ce_override   = false;
    ce_synthesized   = false;
    ce_visibility = Vpublic;
    ce_type       = (reason, Tprim Tstring);
    ce_origin     = name;
  }

and class_var_decl c (env, acc) cv =
  let env, ty =
    match cv.cv_type with
      | None -> env, (Reason.Rwitness (fst cv.cv_id), Tany)
      | Some ty' -> Typing_hint.hint env ty'
  in
  let id = snd cv.cv_id in
  let vis = visibility (snd c.c_name) cv.cv_visibility in
  let ce = { ce_final = true; ce_override = false; ce_synthesized = false;
             ce_visibility = vis; ce_type = ty; ce_origin = (snd c.c_name);
           } in
  let acc = SMap.add id ce acc in
  env, acc

and static_class_var_decl c (env, acc) cv =
  let env, ty =
    match cv.cv_type with
      | None -> env, (Reason.Rwitness (fst cv.cv_id), Tany)
      | Some ty -> Typing_hint.hint env ty in
  let id = snd cv.cv_id in
  let vis = visibility (snd c.c_name) cv.cv_visibility in
  let ce = { ce_final = true; ce_override = false; ce_synthesized = false;
             ce_visibility = vis; ce_type = ty; ce_origin = (snd c.c_name);
           }
  in
  let acc = SMap.add ("$"^id) ce acc in
  if cv.cv_expr = None && (c.c_mode = Ast.Mstrict ||
      c.c_mode = Ast.Mpartial)
  then begin match cv.cv_type with
    | None
    | Some (_, Hmixed)
    | Some (_, Hoption _) -> ()
    | _ -> Errors.add (fst cv.cv_id) "Please assign a value"
  end;
  env, acc

and visibility cid = function
  | Public    -> Vpublic
  | Protected -> Vprotected cid
  | Private   -> Vprivate cid

and method_decl c env m =
  let env, arity, params = Typing.make_params env true 0 m.m_params in
  let env, ret =
    match m.m_ret with
    | None -> env, (Reason.Rwitness (fst m.m_name), Tany)
    | Some ret -> Typing_hint.hint env ret in
  let arity_max =
    if m.m_ddd then 1000 else
    List.length m.m_params
  in
  let env, tparams = lfold Typing.type_param env m.m_tparams in
  let ft = {
    ft_pos = fst m.m_name;
    ft_unsafe    = m.m_unsafe;
    ft_abstract  = m.m_abstract;
    ft_arity_min = arity;
    ft_arity_max = arity_max;
    ft_tparams   = tparams;
    ft_params    = params;
    ft_ret       = ret;
  } in
  let ty = Reason.Rwitness (fst m.m_name), Tfun ft in
  env, ty

and method_check_override c m acc =
  let pos, id = m.m_name in
  let class_pos, class_id = c.c_name in
  let override = SMap.mem "Override" m.m_user_attributes in
  if m.m_visibility = Private && override then
    Errors.add pos ((Utils.strip_ns class_id)^"::"^id
               ^": combining private and override is nonsensical");
  match SMap.get id acc with
    | Some { ce_final = is_final; ce_type = (r, _); _ } ->
      if is_final then
        error_final ~parent:(Reason.to_pos r) ~child:pos;
      false
    | None when override && c.c_kind = Ast.Ctrait -> true
    | None when override ->
      Errors.add pos
          ((Utils.strip_ns class_id)^"::"^id^"() should be an override; \
           no non-private parent definition found \
           or overridden parent is defined in non-<?hh code");
      false
    | None -> false

and method_decl_acc c (env, acc) m =
  let check_override = method_check_override c m acc in
  let env, ty = method_decl c env m in
  let _, id = m.m_name in
  let vis =
    match SMap.get id acc, m.m_visibility with
      | Some { ce_visibility = Vprotected _ as parent_vis; _ }, Protected ->
        parent_vis
    | _ -> visibility (snd c.c_name) m.m_visibility
  in
  let ce = {
    ce_final = m.m_final; ce_override = check_override; ce_synthesized = false;
    ce_visibility = vis; ce_type = ty; ce_origin = snd (c.c_name);
  } in
  let acc = SMap.add id ce acc in
  env, acc

and method_check_trait_overrides c id method_ce =
  if method_ce.ce_override then
    let c_pos, c_name = c.c_name in
    let err_msg =
      ("Method "^(Utils.strip_ns c_name)^"::"^id^" is should be an override \
        per the declaring trait; no non-private parent definition found \
        or overridden parent is defined in non-<?hh code")
    in Errors.add_list [
      c_pos, err_msg;
      (Reason.to_pos (fst method_ce.ce_type)), "Declaration of "^id^"() is here"
    ]

(*****************************************************************************)
(* Dealing with typedefs *)
(*****************************************************************************)

let rec type_typedef_decl_if_missing nenv typedef =
  let pos, tid = typedef.Ast.t_id in
  if Naming_heap.TypedefHeap.mem tid
  then ()
  else
    type_typedef_naming_and_decl nenv typedef

and type_typedef_naming_and_decl nenv tdef =
  let pos, tid = tdef.Ast.t_id in
  let is_abstract =
    match tdef.Ast.t_kind with
    | Ast.Alias x -> false
    | Ast.NewType x -> true
  in
  let params, tcstr, concrete_type =
    Naming.typedef nenv tdef in
  let decl = is_abstract, params, concrete_type in
  let filename = Pos.filename pos in
  let env = Typing_env.empty filename in
  let env = Typing_env.set_mode env tdef.Ast.t_mode in
  let env = Env.set_root env (Typing_deps.Dep.Class tid) in
  let env, params = lfold Typing.type_param env params in
  let env, concrete_type = Typing_hint.hint env concrete_type in
  let env, tcstr =
    match tcstr with
    | None -> env, None
    | Some constraint_type ->
        let env, constraint_type = Typing_hint.hint env constraint_type in
        let sub_type = Typing_ops.sub_type pos Reason.URnewtype_cstr in
        let env = sub_type env constraint_type concrete_type in
        env, Some constraint_type
  in
  let visibility =
    if is_abstract
    then Env.Typedef.Private filename
    else Env.Typedef.Public
  in
  let tdecl = visibility, params, tcstr, concrete_type in
  Env.add_typedef tid tdecl;
  Naming_heap.TypedefHeap.add tid decl;
  ()

(*****************************************************************************)
(* Global constants *)
(*****************************************************************************)

let iconst_decl nenv cst =
  let cst = Naming.global_const nenv cst in
  let _cst_pos, cst_name = cst.cst_name in
  Naming_heap.ConstHeap.add cst_name cst;
  Typing.gconst_decl cst;
  ()

(*****************************************************************************)

let name_and_declare_types_program nenv all_classes prog =
  List.iter begin fun def ->
    match def with
    | Ast.Namespace _
    | Ast.NamespaceUse _ -> assert false
    | Ast.Fun f -> ifun_decl nenv f
    | Ast.Class c ->
      let class_env = {
        nenv = nenv;
        stack = SSet.empty;
        all_classes = all_classes;
      } in
      class_decl_if_missing class_env c
    | Ast.Typedef typedef ->
      type_typedef_decl_if_missing nenv typedef
    | Ast.Stmt _ -> ()
    | Ast.Constant cst ->
        iconst_decl nenv cst
  end prog

let make_env nenv all_classes fn =
  match Parser_heap.ParserHeap.get fn with
  | None -> ()
  | Some prog -> name_and_declare_types_program nenv all_classes prog
