(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module used when we want to figure out what has changed.
 * 1) The user modifies a file
 * 2) We compute the type of the file (cf typing_redecl_service.ml)
 * 3) We compare the old type and the new type of the class (or function)
 *    to see if anything has changed. This is where the code of this module
 *    comes into play. This code compares the old and the new class type
 *    and computes the set of dependencies if something needs to be
 *    either redeclared or checked again.
 *)
(*****************************************************************************)
open Hh_prelude
open Decl_defs
open Typing_deps
open Typing_defs

(*****************************************************************************)
(* Given two classes give back the set of functions or classes that need
 * to be rechecked
 *)
(*****************************************************************************)
module ClassDiff = struct
  let smap_left s1 s2 =
    SMap.fold
      begin
        fun x ty1 diff ->
          let ty2 = SMap.find_opt x s2 in
          match ty2 with
          | Some ty2 ->
            if Poly.( = ) ty1 ty2 then
              diff
            else
              SSet.add x diff
          | None -> SSet.add x diff
      end
      s1
      SSet.empty

  let smap s1 s2 = SSet.union (smap_left s1 s2) (smap_left s2 s1)

  let add_inverted_dep mode build_obj x acc =
    DepSet.union (Typing_deps.get_ideps mode (build_obj x)) acc

  let add_inverted_deps mode acc build_obj xset =
    SSet.fold (add_inverted_dep mode build_obj) xset acc

  let compare mode cid class1 class2 =
    let acc = DepSet.make () in
    let is_unchanged = true in
    (* compare class constants *)
    let consts_diff = smap class1.dc_consts class2.dc_consts in
    let is_unchanged = is_unchanged && SSet.is_empty consts_diff in
    let acc =
      add_inverted_deps mode acc (fun x -> Dep.Const (cid, x)) consts_diff
    in
    (* compare class members *)
    let props_diff = smap class1.dc_props class2.dc_props in
    let is_unchanged = is_unchanged && SSet.is_empty props_diff in
    let acc =
      add_inverted_deps mode acc (fun x -> Dep.Prop (cid, x)) props_diff
    in
    (* compare class static members *)
    let sprops_diff = smap class1.dc_sprops class2.dc_sprops in
    let is_unchanged = is_unchanged && SSet.is_empty sprops_diff in
    let acc =
      add_inverted_deps mode acc (fun x -> Dep.SProp (cid, x)) sprops_diff
    in
    (* compare class methods *)
    let methods_diff = smap class1.dc_methods class2.dc_methods in
    let is_unchanged = is_unchanged && SSet.is_empty methods_diff in
    let acc =
      add_inverted_deps mode acc (fun x -> Dep.Method (cid, x)) methods_diff
    in
    (* compare class static methods *)
    let smethods_diff = smap class1.dc_smethods class2.dc_smethods in
    let is_unchanged = is_unchanged && SSet.is_empty smethods_diff in
    let acc =
      add_inverted_deps mode acc (fun x -> Dep.SMethod (cid, x)) smethods_diff
    in
    (* compare class constructors *)
    let ctor_diff = Poly.( <> ) class1.dc_construct class2.dc_construct in
    let is_unchanged = is_unchanged && not ctor_diff in
    let ctor_ideps = Typing_deps.get_ideps mode (Dep.Constructor cid) in
    let acc =
      if ctor_diff then
        DepSet.union acc ctor_ideps
      else
        acc
    in
    (* compare class type constants *)
    let typeconsts_diff = smap class1.dc_typeconsts class2.dc_typeconsts in
    let is_unchanged = is_unchanged && SSet.is_empty typeconsts_diff in
    let acc =
      add_inverted_deps mode acc (fun x -> Dep.Const (cid, x)) typeconsts_diff
    in
    (acc, is_unchanged)
end

(*****************************************************************************)
(* Given two classes give back the set of functions or classes that need
 * to be rechecked because the type of their member changed
 *)
(*****************************************************************************)
module ClassEltDiff = struct
  open Decl_heap

  let add_inverted_dep mode build_obj x _ acc =
    DepSet.union (Typing_deps.get_ideps mode (build_obj x)) acc

  let add_inverted_deps mode (acc, is_unchanged) build_obj xmap =
    let is_unchanged =
      match is_unchanged with
      | `Unchanged when not @@ SMap.is_empty xmap -> `Changed
      | x -> x
    in
    let acc = SMap.fold (add_inverted_dep mode build_obj) xmap acc in
    (acc, is_unchanged)

  let diff_elts
      (type t)
      (module EltHeap : SharedMem.Heap
        with type key = string * string
         and type value = t)
      ~cid
      ~elts1
      ~elts2
      ~normalize =
    SMap.merge
      begin
        fun name elt1 elt2 ->
          let key = (cid, name) in
          let match1 =
            match elt1 with
            | Some elt -> String.equal elt.elt_origin cid
            | _ -> false
          in
          let match2 =
            match elt2 with
            | Some elt -> String.equal elt.elt_origin cid
            | _ -> false
          in
          if match1 || match2 then
            match (EltHeap.get_old key, EltHeap.get key) with
            | (None, _)
            | (_, None) ->
              Some ()
            | (Some x1, Some x2) ->
              let ty1 = normalize x1 in
              let ty2 = normalize x2 in
              if Poly.( = ) ty1 ty2 then
                None
              else
                Some ()
          else
            None
      end
      elts1
      elts2

  let compare_props mode class1 class2 acc =
    let cid = class1.dc_name in
    let (elts1, elts2) = (class1.dc_props, class2.dc_props) in
    let diff =
      diff_elts
        (module Props)
        ~cid
        ~elts1
        ~elts2
        ~normalize:Decl_pos_utils.NormalizeSig.ty
    in
    add_inverted_deps mode acc (fun x -> Dep.Prop (cid, x)) diff

  let compare_sprops mode class1 class2 acc =
    let cid = class1.dc_name in
    let (elts1, elts2) = (class1.dc_sprops, class2.dc_sprops) in
    let diff =
      diff_elts
        (module StaticProps)
        ~cid
        ~elts1
        ~elts2
        ~normalize:Decl_pos_utils.NormalizeSig.ty
    in
    add_inverted_deps mode acc (fun x -> Dep.SProp (cid, x)) diff

  let compare_meths mode class1 class2 acc =
    let cid = class1.dc_name in
    let (elts1, elts2) = (class1.dc_methods, class2.dc_methods) in
    let diff =
      diff_elts
        (module Methods)
        ~cid
        ~elts1
        ~elts2
        ~normalize:Decl_pos_utils.NormalizeSig.fun_elt
    in
    add_inverted_deps mode acc (fun x -> Dep.Method (cid, x)) diff

  let compare_smeths mode class1 class2 acc =
    let cid = class1.dc_name in
    let (elts1, elts2) = (class1.dc_smethods, class2.dc_smethods) in
    let diff =
      diff_elts
        (module StaticMethods)
        ~cid
        ~elts1
        ~elts2
        ~normalize:Decl_pos_utils.NormalizeSig.fun_elt
    in
    add_inverted_deps mode acc (fun x -> Dep.SMethod (cid, x)) diff

  let compare_cstrs mode class1 class2 =
    let cid = class1.dc_name in
    let match1 =
      match class1.dc_construct with
      | (Some elt, _) -> String.equal elt.elt_origin cid
      | _ -> false
    in
    let match2 =
      match class2.dc_construct with
      | (Some elt, _) -> String.equal elt.elt_origin cid
      | _ -> false
    in
    if match1 || match2 then
      match (Constructors.get_old cid, Constructors.get cid) with
      | (None, _)
      | (_, None) ->
        (Typing_deps.get_ideps mode (Dep.Constructor cid), `Changed)
      | (Some fe1, Some fe2) ->
        let fe1 = Decl_pos_utils.NormalizeSig.fun_elt fe1 in
        let fe2 = Decl_pos_utils.NormalizeSig.fun_elt fe2 in
        if Poly.( = ) fe1 fe2 then
          (DepSet.make (), `Unchanged)
        else
          (Typing_deps.get_ideps mode (Dep.Constructor cid), `Changed)
    else
      (DepSet.make (), `Unchanged)

  let compare mode class1 class2 =
    compare_cstrs mode class1 class2
    |> compare_props mode class1 class2
    |> compare_sprops mode class1 class2
    |> compare_meths mode class1 class2
    |> compare_smeths mode class1 class2
end

let add_changed acc dep = DepSet.add acc (Dep.make dep)

(*****************************************************************************)
(* Determines if there is a "big" difference between two classes
 * What it really means: most of the time, a change in a class doesn't affect
 * the users of the class, recomputing the sub-classes is enough.
 * However, there are some cases, where we really need to re-check all the
 * use cases of a class. For example: if a class doesn't implement an
 * interface anymore, all the subtyping is changed, so we have to recheck
 * all the places where the class was used.
 *)
(*****************************************************************************)
let class_big_diff class1 class2 =
  let class1 = Decl_pos_utils.NormalizeSig.class_type class1 in
  let class2 = Decl_pos_utils.NormalizeSig.class_type class2 in
  let ( <> ) = Poly.( <> ) in
  class1.dc_need_init <> class2.dc_need_init
  || SSet.compare
       class1.dc_deferred_init_members
       class2.dc_deferred_init_members
     <> 0
  || class1.dc_kind <> class2.dc_kind
  || class1.dc_is_xhp <> class2.dc_is_xhp
  || class1.dc_has_xhp_keyword <> class2.dc_has_xhp_keyword
  || class1.dc_const <> class2.dc_const
  || class1.dc_tparams <> class2.dc_tparams
  || SMap.compare compare_subst_context class1.dc_substs class2.dc_substs <> 0
  || SMap.compare
       Typing_defs.compare_decl_ty
       class1.dc_ancestors
       class2.dc_ancestors
     <> 0
  || List.compare Poly.compare class1.dc_req_ancestors class2.dc_req_ancestors
     <> 0
  || SSet.compare
       class1.dc_req_ancestors_extends
       class2.dc_req_ancestors_extends
     <> 0
  || SSet.compare class1.dc_extends class2.dc_extends <> 0
  || SSet.compare class1.dc_xhp_attr_deps class2.dc_xhp_attr_deps <> 0
  || class1.dc_enum_type <> class2.dc_enum_type
  || class1.dc_internal <> class2.dc_internal
  || Option.map ~f:snd class1.dc_module <> Option.map ~f:snd class2.dc_module

let get_extend_deps mode cid_hash acc =
  Typing_deps.get_extend_deps
    ~mode
    ~visited:(VisitedSet.make ())
    ~source_class:cid_hash
    ~acc

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new type signature of "fid" (function identifier).
 *)
(*****************************************************************************)
let get_fun_deps
    ~ctx ~mode old_funs fid ((changed, to_recheck), old_funs_missing) =
  match
    ( SMap.find fid old_funs,
      match Provider_backend.get () with
      | Provider_backend.Rust_provider_backend backend ->
        Rust_provider_backend.Decl.get_fun
          backend
          (Naming_provider.rust_backend_ctx_proxy ctx)
          fid
      | _ -> Decl_heap.Funs.get fid )
  with
  (* Note that we must include all dependencies even if we get the None, None
   * case. Due to the fact we can declare types lazily, there may be no
   * existing declaration in the old Decl_heap that corresponds to a function
   * `foo` in the AST. Then when the user deletes `foo`, the new Decl_heap
   * will also lack a definition of `foo`. Now we must recheck all the use
   * sites of `foo` to make sure there are no dangling references. *)
  | (None, _)
  | (_, None) ->
    let dep = Dep.Fun fid in
    let where_fun_is_used = Typing_deps.get_ideps mode dep in
    let to_recheck = DepSet.union where_fun_is_used to_recheck in
    ((add_changed changed dep, to_recheck), old_funs_missing + 1)
  | (Some fe1, Some fe2) ->
    let fe1 = Decl_pos_utils.NormalizeSig.fun_elt fe1 in
    let fe2 = Decl_pos_utils.NormalizeSig.fun_elt fe2 in
    if Poly.( = ) fe1 fe2 then
      ((changed, to_recheck), old_funs_missing)
    else
      let dep = Dep.Fun fid in
      let where_fun_is_used = Typing_deps.get_ideps mode dep in
      ( (add_changed changed dep, DepSet.union where_fun_is_used to_recheck),
        old_funs_missing )

let get_funs_deps ~ctx old_funs funs =
  let mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    (get_fun_deps ~ctx ~mode old_funs)
    funs
    ((DepSet.make (), DepSet.make ()), 0)

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new typedef
 *)
(*****************************************************************************)
let get_type_deps
    ~ctx ~mode old_types tid ((changed, to_recheck), old_types_missing) =
  match
    ( SMap.find tid old_types,
      match Provider_backend.get () with
      | Provider_backend.Rust_provider_backend backend ->
        Rust_provider_backend.Decl.get_typedef
          backend
          (Naming_provider.rust_backend_ctx_proxy ctx)
          tid
      | _ -> Decl_heap.Typedefs.get tid )
  with
  | (None, _)
  | (_, None) ->
    let dep = Dep.Type tid in
    let where_typedef_was_used = Typing_deps.get_ideps mode dep in
    ( (add_changed changed dep, DepSet.union where_typedef_was_used to_recheck),
      old_types_missing + 1 )
  | (Some tdef1, Some tdef2) ->
    let tdef1 = Decl_pos_utils.NormalizeSig.typedef tdef1 in
    let tdef2 = Decl_pos_utils.NormalizeSig.typedef tdef2 in
    let is_same_signature = Poly.( = ) tdef1 tdef2 in
    if is_same_signature then
      ((changed, to_recheck), old_types_missing)
    else
      let dep = Dep.Type tid in
      let where_type_is_used = Typing_deps.get_ideps mode dep in
      let to_recheck = DepSet.union where_type_is_used to_recheck in
      ((add_changed changed dep, to_recheck), old_types_missing)

let get_types_deps ~ctx old_types types =
  let mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    (get_type_deps ~ctx ~mode old_types)
    types
    ((DepSet.make (), DepSet.make ()), 0)

(*****************************************************************************)
(* Determine which top level definitions have to be rechecked if the constant
 * changed.
 *)
(*****************************************************************************)
let get_gconst_deps
    ~ctx ~mode old_gconsts cst_id ((changed, to_recheck), old_gconsts_missing) =
  let cst1 = SMap.find cst_id old_gconsts in
  let cst2 =
    match Provider_backend.get () with
    | Provider_backend.Rust_provider_backend backend ->
      Rust_provider_backend.Decl.get_gconst
        backend
        (Naming_provider.rust_backend_ctx_proxy ctx)
        cst_id
    | _ -> Decl_heap.GConsts.get cst_id
  in
  match (cst1, cst2) with
  | (None, _)
  | (_, None) ->
    let dep = Dep.GConst cst_id in
    let where_const_is_used = Typing_deps.get_ideps mode dep in
    let to_recheck = DepSet.union where_const_is_used to_recheck in
    let const_name = Typing_deps.get_ideps mode (Dep.GConstName cst_id) in
    ( (add_changed changed dep, DepSet.union const_name to_recheck),
      old_gconsts_missing + 1 )
  | (Some cst1, Some cst2) ->
    let is_same_signature = Poly.( = ) cst1 cst2 in
    if is_same_signature then
      ((changed, to_recheck), old_gconsts_missing)
    else
      let dep = Dep.GConst cst_id in
      let where_type_is_used = Typing_deps.get_ideps mode dep in
      let to_recheck = DepSet.union where_type_is_used to_recheck in
      ((add_changed changed dep, to_recheck), old_gconsts_missing)

let get_gconsts_deps ~ctx old_gconsts gconsts =
  let mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    (get_gconst_deps ~ctx ~mode old_gconsts)
    gconsts
    ((DepSet.make (), DepSet.make ()), 0)

let rule_changed rule1 rule2 =
  match (rule1, rule2) with
  | (MRGlobal, MRGlobal) -> false
  | (MRExact m1, MRExact m2) -> not (String.equal m1 m2)
  | (MRPrefix p1, MRPrefix p2) -> not (String.equal p1 p2)
  | _ -> true

let rules_changed rules1 rules2 =
  match (rules1, rules2) with
  | (None, None) -> false
  | (_, None)
  | (None, _) ->
    true
  | (Some rules_list1, Some rules_list2) ->
    (match List.exists2 rules_list1 rules_list2 ~f:rule_changed with
    | List.Or_unequal_lengths.Ok res -> res
    | List.Or_unequal_lengths.Unequal_lengths -> true)

let get_module_deps
    ~ctx ~mode old_modules mid ((changed, to_recheck), old_modules_missing) =
  match
    ( SMap.find mid old_modules,
      match Provider_backend.get () with
      | Provider_backend.Rust_provider_backend backend ->
        Rust_provider_backend.Decl.get_module
          backend
          (Naming_provider.rust_backend_ctx_proxy ctx)
          mid
      | _ -> Decl_heap.Modules.get mid )
  with
  | (None, _)
  | (_, None) ->
    let dep = Dep.Module mid in
    let where_module_referenced = Typing_deps.get_ideps mode dep in
    let to_recheck = DepSet.union where_module_referenced to_recheck in
    ((add_changed changed dep, to_recheck), old_modules_missing + 1)
  | (Some module1, Some module2) ->
    if
      rules_changed module1.mdt_exports module2.mdt_exports
      || rules_changed module1.mdt_imports module2.mdt_imports
    then
      let dep = Dep.Module mid in
      let where_module_referenced = Typing_deps.get_ideps mode dep in
      let to_recheck = DepSet.union where_module_referenced to_recheck in
      ((add_changed changed dep, to_recheck), old_modules_missing)
    else
      ((changed, to_recheck), old_modules_missing)

let get_modules_deps ~ctx ~old_modules ~modules =
  let mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    (get_module_deps ~ctx ~mode old_modules)
    modules
    ((DepSet.make (), DepSet.make ()), 0)
