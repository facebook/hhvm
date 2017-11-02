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
open Hh_core
open Decl_defs
open Typing_deps

(*****************************************************************************)
(* Given two classes give back the set of functions or classes that need
 * to be rechecked
 *)
(*****************************************************************************)
module ClassDiff = struct

  let smap_left s1 s2 =
    SMap.fold begin fun x ty1 diff ->
      let ty2 = SMap.get x s2 in
      match ty2 with
      | Some ty2 ->
          if ty1 = ty2 then diff else
          SSet.add x diff
      | None ->
          SSet.add x diff
    end s1 SSet.empty

  let smap s1 s2 =
    SSet.union (smap_left s1 s2) (smap_left s2 s1)

  let add_inverted_dep build_obj x acc =
    DepSet.union (Typing_deps.get_ideps (build_obj x)) acc

  let add_inverted_deps acc build_obj xset =
    SSet.fold (add_inverted_dep build_obj) xset acc

  let compare cid class1 class2 =
    let acc = DepSet.empty in
    let is_unchanged = true in

    (* compare class constants *)
    let consts_diff = smap class1.dc_consts class2.dc_consts in
    let is_unchanged = is_unchanged && SSet.is_empty consts_diff in
    let acc = add_inverted_deps acc (fun x -> Dep.Const (cid, x)) consts_diff in

    (* compare class members *)
    let props_diff = smap class1.dc_props class2.dc_props in
    let is_unchanged = is_unchanged && SSet.is_empty props_diff in
    let acc = add_inverted_deps acc (fun x -> Dep.Prop (cid, x)) props_diff in

    (* compare class static members *)
    let sprops_diff = smap class1.dc_sprops class2.dc_sprops in
    let is_unchanged = is_unchanged && SSet.is_empty sprops_diff in
    let acc = add_inverted_deps acc (fun x -> Dep.SProp (cid, x)) sprops_diff in

    (* compare class methods *)
    let methods_diff = smap class1.dc_methods class2.dc_methods in
    let is_unchanged = is_unchanged && SSet.is_empty methods_diff in
    let acc = add_inverted_deps acc (fun x -> Dep.Method (cid, x)) methods_diff in

    (* compare class static methods *)
    let smethods_diff = smap class1.dc_smethods class2.dc_smethods in
    let is_unchanged = is_unchanged && SSet.is_empty smethods_diff in
    let acc = add_inverted_deps acc (fun x -> Dep.SMethod (cid, x)) smethods_diff in

    (* compare class constructors *)
    let cstr_diff = class1.dc_construct <> class2.dc_construct in
    let is_unchanged = is_unchanged && not cstr_diff in
    let cstr_ideps = Typing_deps.get_ideps (Dep.Cstr cid) in
    let acc = if cstr_diff then DepSet.union acc cstr_ideps else acc in

    (* compare class type constants *)
    let typeconsts_diff = smap class1.dc_typeconsts class2.dc_typeconsts in
    let is_unchanged = is_unchanged && SSet.is_empty typeconsts_diff in
    let acc =
      add_inverted_deps acc (fun x -> Dep.Const (cid, x)) typeconsts_diff in

    acc, is_unchanged

end

(*****************************************************************************)
(* Given two classes give back the set of functions or classes that need
 * to be rechecked because the type of their member changed
 *)
(*****************************************************************************)
module ClassEltDiff = struct
  open Decl_heap

  let add_inverted_dep build_obj x _ acc =
    DepSet.union (Typing_deps.get_ideps (build_obj x)) acc

  let add_inverted_deps (acc, is_unchanged) build_obj xmap =
    let is_unchanged = match is_unchanged with
      | `Unchanged when not @@ SMap.is_empty xmap -> `Changed
      | x -> x
    in
    let acc = SMap.fold (add_inverted_dep build_obj) xmap acc in
    acc, is_unchanged

  let diff_elts (type t) (module EltHeap: SharedMem.NoCache
      with type key = string * string
       and type t = t
    ) ~cid ~elts1 ~elts2 ~normalize =
    SMap.merge begin fun name elt1 elt2 ->
      let key = (cid, name) in
      match elt1, elt2 with
      | Some elt, _ | _, Some elt when elt.elt_origin = cid ->
        begin
          match EltHeap.get_old key, EltHeap.get key with
          | None, _ | _, None -> Some ()
          | Some x1, Some x2 ->
            let ty1 = normalize x1 in
            let ty2 = normalize x2 in
            if ty1 = ty2
            then None
            else Some ()
        end
      | _ -> None
    end elts1 elts2

  let compare_props class1 class2 acc =
    let cid = class1.dc_name in
    let elts1, elts2 = class1.dc_props, class2.dc_props in
    let diff = diff_elts (module Props) ~cid ~elts1 ~elts2
        ~normalize:Decl_pos_utils.NormalizeSig.ty in
    add_inverted_deps acc (fun x -> Dep.Prop (cid, x)) diff

  let compare_sprops class1 class2 acc =
    let cid = class1.dc_name in
    let elts1, elts2 = class1.dc_sprops, class2.dc_sprops in
    let diff = diff_elts (module StaticProps) ~cid ~elts1 ~elts2
        ~normalize:Decl_pos_utils.NormalizeSig.ty in
    add_inverted_deps acc (fun x -> Dep.SProp (cid, x)) diff

  let compare_meths class1 class2 acc =
    let cid = class1.dc_name in
    let elts1, elts2 = class1.dc_methods, class2.dc_methods in
    let diff = diff_elts (module Methods) ~cid ~elts1 ~elts2
        ~normalize:Decl_pos_utils.NormalizeSig.fun_type in
    add_inverted_deps acc (fun x -> Dep.Method (cid, x)) diff

  let compare_smeths class1 class2 acc =
    let cid = class1.dc_name in
    let elts1, elts2 = class1.dc_smethods, class2.dc_smethods in
    let diff = diff_elts (module StaticMethods) ~cid ~elts1 ~elts2
        ~normalize:Decl_pos_utils.NormalizeSig.fun_type in
    add_inverted_deps acc (fun x -> Dep.SMethod (cid, x)) diff

  let compare_cstrs class1 class2 =
    let cid = class1.dc_name in
    match class1.dc_construct, class2.dc_construct with
    | (Some elt, _), _
    | _, (Some elt, _) when elt.elt_origin = cid ->
      begin
        match Constructors.get_old cid, Constructors.get cid with
        | None, _ | _, None -> Typing_deps.get_ideps (Dep.Cstr cid), `Changed
        | Some ft1, Some ft2 ->
            let ft1 = Decl_pos_utils.NormalizeSig.fun_type ft1 in
            let ft2 = Decl_pos_utils.NormalizeSig.fun_type ft2 in
            if ft1 = ft2
            then DepSet.empty, `Unchanged
            else Typing_deps.get_ideps (Dep.Cstr cid), `Changed
      end
    | _ -> DepSet.empty, `Unchanged

  let compare class1 class2 =
    compare_cstrs class1 class2
    |> compare_props class1 class2
    |> compare_sprops class1 class2
    |> compare_meths class1 class2
    |> compare_smeths class1 class2
end

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
  class1.dc_need_init <> class2.dc_need_init ||
  SSet.compare class1.dc_deferred_init_members class2.dc_deferred_init_members <> 0 ||
  class1.dc_members_fully_known <> class2.dc_members_fully_known ||
  class1.dc_kind <> class2.dc_kind ||
  class1.dc_tparams <> class2.dc_tparams ||
  SMap.compare class1.dc_substs class2.dc_substs <> 0 ||
  SMap.compare class1.dc_ancestors class2.dc_ancestors <> 0 ||
  List.compare ~cmp:Pervasives.compare
    class1.dc_req_ancestors class2.dc_req_ancestors <> 0 ||
  SSet.compare class1.dc_req_ancestors_extends class2.dc_req_ancestors_extends <> 0 ||
  SSet.compare class1.dc_extends class2.dc_extends <> 0 ||
  class1.dc_enum_type <> class2.dc_enum_type ||
  (* due to, e.g. switch exhaustiveness checks, a change in an enum's
   * constant set is a "big" difference *)
    (class1.dc_enum_type <> None &&
       not (SSet.is_empty (ClassDiff.smap class1.dc_consts class2.dc_consts)))

(*****************************************************************************)
(* Given a class name adds all the subclasses, we need a "trace" to follow
 * what we have already added.
 *)
(*****************************************************************************)
let rec get_extend_deps_ trace cid_hash to_redecl =
  if DepSet.mem !trace cid_hash
  then to_redecl
  else begin
    trace := DepSet.add !trace cid_hash;
    let cid_hash = Typing_deps.Dep.extends_of_class cid_hash in
    let ideps = Typing_deps.get_ideps_from_hash cid_hash in
    DepSet.fold ~f:begin fun obj acc ->
      if Typing_deps.Dep.is_class obj
      then
        let to_redecl = DepSet.add acc obj in
        get_extend_deps_ trace obj to_redecl
      else acc
    end ideps ~init:to_redecl
  end

(*****************************************************************************)
(* GET EVERYTHING, don't think, don't try to be subtle, don't try to be
 * smart what so ever, just get EVERYTHING that ever used the class "cid"
 * (cid = class identifier).
 * Hence the name "get_bazooka".
 *)
(*****************************************************************************)
and get_all_dependencies trace cid (to_redecl, to_recheck) =
  let bazooka = Typing_deps.get_bazooka (Dep.Class cid) in
  let to_redecl = DepSet.union bazooka to_redecl in
  let to_recheck = DepSet.union bazooka to_recheck in
  let cid_hash = Typing_deps.Dep.make (Dep.Class cid) in
  let to_redecl = get_extend_deps_ trace cid_hash to_redecl in
  to_redecl, to_recheck

let get_extend_deps cid_hash to_redecl =
  get_extend_deps_ (ref DepSet.empty) cid_hash to_redecl

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new type signature of "fid" (function identifier).
*)
(*****************************************************************************)
let get_fun_deps old_funs fid (to_redecl, to_recheck) =
  match SMap.find_unsafe fid old_funs, Decl_heap.Funs.get fid with
  (* Note that we must include all dependencies even if we get the None, None
   * case. Due to the fact we can declare types lazily, there may be no
   * existing declaration in the old Decl_heap that corresponds to a function
   * `foo` in the AST. Then when the user deletes `foo`, the new Decl_heap
   * will also lack a definition of `foo`. Now we must recheck all the use
   * sites of `foo` to make sure there are no dangling references. *)
  | None, _ | _, None ->
      let where_fun_is_used = Typing_deps.get_bazooka (Dep.Fun fid) in
      let to_recheck = DepSet.union where_fun_is_used to_recheck in
      let fun_name = Typing_deps.get_bazooka (Dep.FunName fid) in
      DepSet.union fun_name to_redecl, DepSet.union fun_name to_recheck
  | Some fty1, Some fty2 ->
      let fty1 = Decl_pos_utils.NormalizeSig.fun_type fty1 in
      let fty2 = Decl_pos_utils.NormalizeSig.fun_type fty2 in
      let is_same_signature = fty1 = fty2 in
      if is_same_signature
      then to_redecl, to_recheck
      else
        (* No need to add Dep.FunName stuff here -- we found a function with the
         * right name already otherwise we'd be in the None case above. *)
        let where_fun_is_used = Typing_deps.get_bazooka (Dep.Fun fid) in
        to_redecl, DepSet.union where_fun_is_used to_recheck

let get_funs_deps old_funs funs =
  SSet.fold (get_fun_deps old_funs) funs (DepSet.empty, DepSet.empty)

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new typedef
*)
(*****************************************************************************)
let get_type_deps old_types tid to_recheck =
  match SMap.find_unsafe tid old_types, Decl_heap.Typedefs.get tid with
  | None, _ | _, None ->
      let bazooka = Typing_deps.get_bazooka (Dep.Class tid) in
      DepSet.union bazooka to_recheck
  | Some tdef1, Some tdef2 ->
      let tdef1 = Decl_pos_utils.NormalizeSig.typedef tdef1 in
      let tdef2 = Decl_pos_utils.NormalizeSig.typedef tdef2 in
      let is_same_signature = tdef1 = tdef2 in
      if is_same_signature
      then to_recheck
      else
        let where_type_is_used = Typing_deps.get_ideps (Dep.Class tid) in
        let to_recheck = DepSet.union where_type_is_used to_recheck in
        to_recheck

let get_types_deps old_types types =
  SSet.fold (get_type_deps old_types) types DepSet.empty

(*****************************************************************************)
(* Determine which top level definitions have to be rechecked if the constant
 * changed.
 *)
(*****************************************************************************)
let get_gconst_deps old_gconsts cst_id (to_redecl, to_recheck) =
  let cst1 = SMap.find_unsafe cst_id old_gconsts in
  let cst2 = Decl_heap.GConsts.get cst_id in
  match cst1, cst2 with
  | None, _ | _, None ->
      let where_const_is_used = Typing_deps.get_bazooka (Dep.GConst cst_id) in
      let to_recheck = DepSet.union where_const_is_used to_recheck in
      let const_name = Typing_deps.get_bazooka (Dep.GConstName cst_id) in
      DepSet.union const_name to_redecl, DepSet.union const_name to_recheck
  | Some cst1, Some cst2 ->
      let is_same_signature = cst1 = cst2 in
      if is_same_signature
      then to_redecl, to_recheck
      else
        let where_type_is_used = Typing_deps.get_ideps (Dep.GConst cst_id) in
        let to_recheck = DepSet.union where_type_is_used to_recheck in
        to_redecl, to_recheck

let get_gconsts_deps old_gconsts gconsts =
  SSet.fold (get_gconst_deps old_gconsts) gconsts (DepSet.empty, DepSet.empty)

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new type signature of "cid" (class identifier).
*)
(*****************************************************************************)
let get_class_deps old_classes new_classes trace cid (to_redecl, to_recheck) =
  match SMap.find_unsafe cid old_classes, SMap.find_unsafe cid new_classes with
  | None, _ | _, None ->
      get_all_dependencies trace cid (to_redecl, to_recheck)
  | Some class1, Some class2 when class_big_diff class1 class2 ->
      get_all_dependencies trace cid (to_redecl, to_recheck)
  | Some class1, Some class2 ->
      let nclass1 = Decl_pos_utils.NormalizeSig.class_type class1 in
      let nclass2 = Decl_pos_utils.NormalizeSig.class_type class2 in
      let deps, is_unchanged = ClassDiff.compare cid nclass1 nclass2 in
      let cid_hash = Typing_deps.Dep.make (Dep.Class cid) in
      let to_redecl, to_recheck =
        if is_unchanged
        then
          let _, is_unchanged = ClassDiff.compare cid class1 class2 in
          if is_unchanged
          then to_redecl, to_recheck
          else
            (* If we reach this case it means that class1 and class2
             * have the same signatures, but that some of their
             * positions differ. We therefore must redeclare the sub-classes
             * but not recheck them.
            *)
            let to_redecl = get_extend_deps_ trace cid_hash to_redecl in
            to_redecl, to_recheck
        else
          let to_redecl = get_extend_deps_ trace cid_hash to_redecl in
          let to_recheck = DepSet.union to_redecl to_recheck in
          DepSet.union deps to_redecl, DepSet.union deps to_recheck
      in

      (* This adds additional files to recheck if the type signature of a class
       * element has changed. We do not require adding any additional redecls
       * because the type is not folded in anyway so it won't affect any other
       * classes.
       *)
      let deps, _ = ClassEltDiff.compare class1 class2 in
      (* TODO: should not need to add to to_redecl *)
      DepSet.union deps to_redecl, DepSet.union deps to_recheck

let get_classes_deps old_classes new_classes classes =
  SSet.fold
    (get_class_deps old_classes new_classes (ref DepSet.empty))
    classes
    (DepSet.empty, DepSet.empty)
