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
    let acc = DepSet.make mode in
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
    let cstr_diff = Poly.( <> ) class1.dc_construct class2.dc_construct in
    let is_unchanged = is_unchanged && not cstr_diff in
    let cstr_ideps = Typing_deps.get_ideps mode (Dep.Cstr cid) in
    let acc =
      if cstr_diff then
        DepSet.union acc cstr_ideps
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
      (module EltHeap : SharedMem.NoCache
        with type key = string * string
         and type t = t)
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
        (Typing_deps.get_ideps mode (Dep.Cstr cid), `Changed)
      | (Some fe1, Some fe2) ->
        let fe1 = Decl_pos_utils.NormalizeSig.fun_elt fe1 in
        let fe2 = Decl_pos_utils.NormalizeSig.fun_elt fe2 in
        if Poly.( = ) fe1 fe2 then
          (DepSet.make mode, `Unchanged)
        else
          (Typing_deps.get_ideps mode (Dep.Cstr cid), `Changed)
    else
      (DepSet.make mode, `Unchanged)

  let compare mode class1 class2 =
    compare_cstrs mode class1 class2
    |> compare_props mode class1 class2
    |> compare_sprops mode class1 class2
    |> compare_meths mode class1 class2
    |> compare_smeths mode class1 class2
end

let add_changed mode acc dep = DepSet.add acc (Dep.make (hash_mode mode) dep)

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
  || class1.dc_members_fully_known <> class2.dc_members_fully_known
  || class1.dc_kind <> class2.dc_kind
  || class1.dc_is_xhp <> class2.dc_is_xhp
  || class1.dc_has_xhp_keyword <> class2.dc_has_xhp_keyword
  || class1.dc_const <> class2.dc_const
  || class1.dc_is_disposable <> class2.dc_is_disposable
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

and get_all_dependencies ~mode trace cid (changed, to_redecl, to_recheck) =
  let dep = Dep.Type cid in
  let cid_hash = Typing_deps.(Dep.make (hash_mode mode) dep) in
  (* Why can't we just use `Typing_deps.get_ideps dep` here? See test case
     hphp/hack/test/integration_ml/saved_state/test_changed_type_in_base_class.ml
     for an example. *)
  let where_class_and_subclasses_were_used =
    Typing_deps.add_all_deps mode (DepSet.singleton mode cid_hash)
  in
  let to_recheck =
    DepSet.union where_class_and_subclasses_were_used to_recheck
  in
  let to_redecl =
    Typing_deps.get_extend_deps
      ~mode
      ~visited:trace
      ~source_class:cid_hash
      ~acc:to_redecl
  in
  (add_changed mode changed dep, to_redecl, to_recheck)

let get_extend_deps mode cid_hash to_redecl =
  Typing_deps.get_extend_deps
    ~mode
    ~visited:(VisitedSet.make mode)
    ~source_class:cid_hash
    ~acc:to_redecl

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new type signature of "fid" (function identifier).
 *)
(*****************************************************************************)
let get_fun_deps ~mode old_funs fid (changed, to_redecl, to_recheck) =
  match (SMap.find fid old_funs, Decl_heap.Funs.get fid) with
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
    (add_changed mode changed dep, to_redecl, to_recheck)
  | (Some fe1, Some fe2) ->
    let fe1 = Decl_pos_utils.NormalizeSig.fun_elt fe1 in
    let fe2 = Decl_pos_utils.NormalizeSig.fun_elt fe2 in
    if Poly.( = ) fe1 fe2 then
      (changed, to_redecl, to_recheck)
    else
      let dep = Dep.Fun fid in
      let where_fun_is_used = Typing_deps.get_ideps mode dep in
      ( add_changed mode changed dep,
        to_redecl,
        DepSet.union where_fun_is_used to_recheck )

let get_funs_deps ~ctx old_funs funs =
  let mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    (get_fun_deps ~mode old_funs)
    funs
    (DepSet.make mode, DepSet.make mode, DepSet.make mode)

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new typedef
 *)
(*****************************************************************************)
let get_type_deps ~mode old_types tid (changed, to_recheck) =
  match (SMap.find tid old_types, Decl_heap.Typedefs.get tid) with
  | (None, _)
  | (_, None) ->
    let dep = Dep.Type tid in
    let where_typedef_was_used = Typing_deps.get_ideps mode dep in
    ( add_changed mode changed dep,
      DepSet.union where_typedef_was_used to_recheck )
  | (Some tdef1, Some tdef2) ->
    let tdef1 = Decl_pos_utils.NormalizeSig.typedef tdef1 in
    let tdef2 = Decl_pos_utils.NormalizeSig.typedef tdef2 in
    let is_same_signature = Poly.( = ) tdef1 tdef2 in
    if is_same_signature then
      (changed, to_recheck)
    else
      let dep = Dep.Type tid in
      let where_type_is_used = Typing_deps.get_ideps mode dep in
      let to_recheck = DepSet.union where_type_is_used to_recheck in
      (add_changed mode changed dep, to_recheck)

let get_types_deps ~ctx old_types types =
  let mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    (get_type_deps ~mode old_types)
    types
    (DepSet.make mode, DepSet.make mode)

(*****************************************************************************)
(* Determine which top level definitions have to be rechecked if the constant
 * changed.
 *)
(*****************************************************************************)
let get_gconst_deps ~mode old_gconsts cst_id (changed, to_redecl, to_recheck) =
  let cst1 = SMap.find cst_id old_gconsts in
  let cst2 = Decl_heap.GConsts.get cst_id in
  match (cst1, cst2) with
  | (None, _)
  | (_, None) ->
    let dep = Dep.GConst cst_id in
    let where_const_is_used = Typing_deps.get_ideps mode dep in
    let to_recheck = DepSet.union where_const_is_used to_recheck in
    let const_name = Typing_deps.get_ideps mode (Dep.GConstName cst_id) in
    (add_changed mode changed dep, to_redecl, DepSet.union const_name to_recheck)
  | (Some cst1, Some cst2) ->
    let is_same_signature = Poly.( = ) cst1 cst2 in
    if is_same_signature then
      (changed, to_redecl, to_recheck)
    else
      let dep = Dep.GConst cst_id in
      let where_type_is_used = Typing_deps.get_ideps mode dep in
      let to_recheck = DepSet.union where_type_is_used to_recheck in
      (add_changed mode changed dep, to_redecl, to_recheck)

let get_gconsts_deps ~ctx old_gconsts gconsts =
  let mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    (get_gconst_deps ~mode old_gconsts)
    gconsts
    (DepSet.make mode, DepSet.make mode, DepSet.make mode)

let shallow_decl_enabled (ctx : Provider_context.t) : bool =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new type signature of "cid" (class identifier).
 *)
(*****************************************************************************)
let get_class_deps
    ctx old_classes new_classes trace cid (changed, to_redecl, to_recheck) =
  let mode = Provider_context.get_deps_mode ctx in
  match (SMap.find cid old_classes, SMap.find cid new_classes) with
  | _ when shallow_decl_enabled ctx ->
    get_all_dependencies ~mode trace cid (changed, to_redecl, to_recheck)
  | (None, _)
  | (_, None) ->
    get_all_dependencies ~mode trace cid (changed, to_redecl, to_recheck)
  | (Some class1, Some class2) when class_big_diff class1 class2 ->
    get_all_dependencies ~mode trace cid (changed, to_redecl, to_recheck)
  | (Some class1, Some class2) ->
    let nclass1 = Decl_pos_utils.NormalizeSig.class_type class1 in
    let nclass2 = Decl_pos_utils.NormalizeSig.class_type class2 in
    let (deps, is_unchanged) = ClassDiff.compare mode cid nclass1 nclass2 in
    let dep = Dep.Type cid in
    let cid_hash = Typing_deps.(Dep.make (hash_mode mode) dep) in
    let (changed, to_redecl, to_recheck) =
      if is_unchanged then
        let (_, is_unchanged) = ClassDiff.compare mode cid class1 class2 in
        if is_unchanged then
          (changed, to_redecl, to_recheck)
        else
          (* If we reach this case it means that class1 and class2
           * have the same signatures, but that some of their
           * positions differ. We therefore must redeclare the sub-classes
           * but not recheck them.
           *)
          let to_redecl =
            Typing_deps.get_extend_deps
              ~mode
              ~visited:trace
              ~source_class:cid_hash
              ~acc:to_redecl
          in
          (changed, to_redecl, to_recheck)
      else
        let to_redecl =
          Typing_deps.get_extend_deps
            ~mode
            ~visited:trace
            ~source_class:cid_hash
            ~acc:to_redecl
        in
        let to_recheck = DepSet.union to_redecl to_recheck in
        let to_recheck =
          DepSet.union
            (Typing_deps.get_ideps mode (Dep.AllMembers cid))
            to_recheck
        in
        ( add_changed mode changed dep,
          DepSet.union deps to_redecl,
          DepSet.union deps to_recheck )
    in
    (* This adds additional files to recheck if the type signature of a class
     * element has changed. We do not require adding any additional redecls
     * because the type is not folded in any way so it won't affect any other
     * classes.
     *)
    let (deps, is_changed) = ClassEltDiff.compare mode class1 class2 in
    let is_changed = phys_equal is_changed `Changed in
    let changed =
      if is_changed then
        add_changed mode changed dep
      else
        changed
    in
    (* If just the type of an element has changed, we don't need to
       _redeclare_ descendants (the type is not copied, so descendant
       declarations do not need recomputing), but we do need to recheck them
       to verify that their declarations are still valid. Running type
       inference on them and rechecking method bodies may not be necessary,
       but decl-validation and typechecking happen in the same step. *)
    let to_recheck =
      if is_changed then
        Typing_deps.get_extend_deps
          ~mode
          ~visited:trace
          ~source_class:cid_hash
          ~acc:to_recheck
      else
        to_recheck
    in
    (changed, to_redecl, DepSet.union deps to_recheck)

let get_classes_deps ~ctx old_classes new_classes classes =
  let mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    (get_class_deps ctx old_classes new_classes (VisitedSet.make mode))
    classes
    (DepSet.make mode, DepSet.make mode, DepSet.make mode)

(*****************************************************************************)
(* Determine which top level definitions have to be rechecked if the record
 * changed.
 *)
(*****************************************************************************)
let get_record_def_deps
    ~mode old_record_defs rdid (changed, to_redecl, to_recheck) =
  match (SMap.find rdid old_record_defs, Decl_heap.RecordDefs.get rdid) with
  | (None, _)
  | (_, None) ->
    let dep = Dep.Type rdid in
    let where_record_is_used = Typing_deps.get_ideps mode dep in
    let to_recheck = DepSet.union where_record_is_used to_recheck in
    (add_changed mode changed dep, to_redecl, to_recheck)
  | (Some rd1, Some rd2) ->
    if Poly.( = ) rd1 rd2 then
      (changed, to_redecl, to_recheck)
    else
      let dep = Dep.Type rdid in
      let where_record_is_used = Typing_deps.get_ideps mode dep in
      let to_recheck = DepSet.union where_record_is_used to_recheck in
      (add_changed mode changed dep, to_redecl, to_recheck)

let get_record_defs_deps ~ctx old_record_defs record_defs =
  let mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    (get_record_def_deps ~mode old_record_defs)
    record_defs
    (DepSet.make mode, DepSet.make mode, DepSet.make mode)
