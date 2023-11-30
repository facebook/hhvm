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

module VersionedNames = struct
  type t = {
    old_names: FileInfo.names;
    new_names: FileInfo.names;
  }
  [@@deriving show]

  let empty : t =
    { old_names = FileInfo.empty_names; new_names = FileInfo.empty_names }

  let make_unchanged (names : FileInfo.names) : t =
    { old_names = names; new_names = names }

  let merge left right : t =
    let { old_names = left_old_names; new_names = left_new_names } = left in
    let { old_names = right_old_names; new_names = right_new_names } = right in
    {
      old_names = FileInfo.merge_names left_old_names right_old_names;
      new_names = FileInfo.merge_names left_new_names right_new_names;
    }
end

module VersionedSSet = struct
  type t = {
    old: SSet.t;
    new_: SSet.t;
  }

  type diff = {
    removed: SSet.t;
    kept: SSet.t;
    added: SSet.t;
  }

  let empty = { old = SSet.empty; new_ = SSet.empty }

  let project (project : FileInfo.names -> SSet.t) (names : VersionedNames.t) :
      t =
    let { VersionedNames.old_names; new_names } = names in
    { old = project old_names; new_ = project new_names }

  let get_classes = project (fun names -> names.FileInfo.n_classes)

  let merge (left : t) (right : t) : t =
    let { old = old_left; new_ = new_left } = left in
    let { old = old_right; new_ = new_right } = right in
    {
      old = SSet.union old_left old_right;
      new_ = SSet.union new_left new_right;
    }

  let empty_diff =
    { removed = SSet.empty; kept = SSet.empty; added = SSet.empty }

  let diff { old; new_ } : diff =
    let diff =
      SSet.fold
        (fun old_name diff ->
          match SSet.find_opt old_name new_ with
          | None -> { diff with removed = SSet.add old_name diff.removed }
          | Some n -> { diff with kept = SSet.add n diff.kept })
        old
        empty_diff
    in
    { diff with added = SSet.diff new_ old }
end

module VersionedFileInfo = struct
  type t = {
    funs: VersionedSSet.t;
    types: VersionedSSet.t;
    gconsts: VersionedSSet.t;
    modules: VersionedSSet.t;
  }

  let transpose (names : VersionedNames.t) : t =
    let { VersionedNames.old_names; new_names } = names in
    let {
      FileInfo.n_funs = old_funs;
      n_classes = _;
      n_types = old_types;
      n_consts = old_gconsts;
      n_modules = old_modules;
    } =
      old_names
    in
    let {
      FileInfo.n_funs = new_funs;
      n_classes = _;
      n_types = new_types;
      n_consts = new_gconsts;
      n_modules = new_modules;
    } =
      new_names
    in
    {
      funs = { VersionedSSet.old = old_funs; new_ = new_funs };
      types = { VersionedSSet.old = old_types; new_ = new_types };
      gconsts = { VersionedSSet.old = old_gconsts; new_ = new_gconsts };
      modules = { VersionedSSet.old = old_modules; new_ = new_modules };
    }

  module Diff = struct
    type t = {
      funs: VersionedSSet.diff;
      types: VersionedSSet.diff;
      gconsts: VersionedSSet.diff;
      modules: VersionedSSet.diff;
    }
  end

  let diff (t : t) : Diff.t =
    let { funs; types; gconsts; modules } = t in
    {
      Diff.funs = VersionedSSet.diff funs;
      types = VersionedSSet.diff types;
      gconsts = VersionedSSet.diff gconsts;
      modules = VersionedSSet.diff modules;
    }

  let diff_names (names : VersionedNames.t) : Diff.t =
    names |> transpose |> diff
end

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

  let compare class1 class2 =
    let is_unchanged = true in
    (* compare class constants *)
    let consts_diff = smap class1.dc_consts class2.dc_consts in
    let is_unchanged = is_unchanged && SSet.is_empty consts_diff in
    (* compare class members *)
    let props_diff = smap class1.dc_props class2.dc_props in
    let is_unchanged = is_unchanged && SSet.is_empty props_diff in
    (* compare class static members *)
    let sprops_diff = smap class1.dc_sprops class2.dc_sprops in
    let is_unchanged = is_unchanged && SSet.is_empty sprops_diff in
    (* compare class methods *)
    let methods_diff = smap class1.dc_methods class2.dc_methods in
    let is_unchanged = is_unchanged && SSet.is_empty methods_diff in
    (* compare class static methods *)
    let smethods_diff = smap class1.dc_smethods class2.dc_smethods in
    let is_unchanged = is_unchanged && SSet.is_empty smethods_diff in
    (* compare class constructors *)
    let ctor_diff = Poly.( <> ) class1.dc_construct class2.dc_construct in
    let is_unchanged = is_unchanged && not ctor_diff in
    (* compare class type constants *)
    let typeconsts_diff = smap class1.dc_typeconsts class2.dc_typeconsts in
    let is_unchanged = is_unchanged && SSet.is_empty typeconsts_diff in
    is_unchanged
end

(*****************************************************************************)
(* Given two classes give back the set of functions or classes that need
 * to be rechecked because the type of their member changed
 *)
(*****************************************************************************)
module ClassEltDiff = struct
  open Decl_heap

  let acc_diff is_unchanged xmap =
    let is_unchanged =
      match is_unchanged with
      | `Unchanged when not @@ SMap.is_empty xmap -> `Changed
      | x -> x
    in
    is_unchanged

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

  let compare_props class1 class2 acc =
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
    acc_diff acc diff

  let compare_sprops class1 class2 acc =
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
    acc_diff acc diff

  let compare_meths class1 class2 acc =
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
    acc_diff acc diff

  let compare_smeths class1 class2 acc =
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
    acc_diff acc diff

  let compare_cstrs class1 class2 =
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
        `Changed
      | (Some fe1, Some fe2) ->
        let fe1 = Decl_pos_utils.NormalizeSig.fun_elt fe1 in
        let fe2 = Decl_pos_utils.NormalizeSig.fun_elt fe2 in
        if Poly.( = ) fe1 fe2 then
          `Unchanged
        else
          `Changed
    else
      `Unchanged

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

let add_fanout make_dep mode id fanout_acc =
  Fanout.add_fanout_of mode (make_dep id) fanout_acc

let add_fun_fanout = add_fanout (fun id -> Dep.Fun id)

let add_type_fanout = add_fanout (fun id -> Dep.Type id)

let add_gconst_fanout mode id fanout =
  fanout
  |> add_fanout (fun id -> Dep.GConst id) mode id
  |> add_fanout (fun id -> Dep.GConstName id) mode id

let add_module_fanout = add_fanout (fun id -> Dep.Module id)

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new type signature of "fid" (function identifier).
 *)
(*****************************************************************************)

let get_fun_deps ~ctx ~mode old_funs fid (fanout_acc, old_funs_missing) =
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
    let fanout_acc = add_fun_fanout mode fid fanout_acc in
    (fanout_acc, old_funs_missing + 1)
  | (Some fe1, Some fe2) ->
    let fe1 = Decl_pos_utils.NormalizeSig.fun_elt fe1 in
    let fe2 = Decl_pos_utils.NormalizeSig.fun_elt fe2 in
    if Poly.( = ) fe1 fe2 then
      (fanout_acc, old_funs_missing)
    else
      let fanout_acc = add_fun_fanout mode fid fanout_acc in
      (fanout_acc, old_funs_missing)

let get_funs_deps ~ctx old_funs (funs : VersionedSSet.diff) =
  let mode = Provider_context.get_deps_mode ctx in
  let { VersionedSSet.removed; kept; added } = funs in
  let fanout =
    Fanout.empty
    |> SSet.fold (add_fun_fanout mode) removed
    |> SSet.fold (add_fun_fanout mode) added
  in
  SSet.fold (get_fun_deps ~ctx ~mode old_funs) kept (fanout, 0)

(*****************************************************************************)
(* Determine which functions/classes have to be rechecked after comparing
 * the old and the new typedef
 *)
(*****************************************************************************)
let get_type_deps ~ctx ~mode old_types tid (fanout_acc, old_types_missing) =
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
    let fanout_acc = add_type_fanout mode tid fanout_acc in
    (fanout_acc, old_types_missing + 1)
  | (Some tdef1, Some tdef2) ->
    let tdef1 = Decl_pos_utils.NormalizeSig.typedef tdef1 in
    let tdef2 = Decl_pos_utils.NormalizeSig.typedef tdef2 in
    let is_same_signature = Poly.( = ) tdef1 tdef2 in
    if is_same_signature then
      (fanout_acc, old_types_missing)
    else
      let fanout_acc = add_type_fanout mode tid fanout_acc in
      (fanout_acc, old_types_missing)

let get_types_deps ~ctx old_types (types : VersionedSSet.diff) =
  let mode = Provider_context.get_deps_mode ctx in
  let { VersionedSSet.removed; kept; added } = types in
  let fanout =
    Fanout.empty
    |> SSet.fold (add_type_fanout mode) removed
    |> SSet.fold (add_type_fanout mode) added
  in
  SSet.fold (get_type_deps ~ctx ~mode old_types) kept (fanout, 0)

(*****************************************************************************)
(* Determine which top level definitions have to be rechecked if the constant
 * changed.
 *)
(*****************************************************************************)
let get_gconst_deps
    ~ctx ~mode old_gconsts cst_id (fanout_acc, old_gconsts_missing) =
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
    let fanout_acc = add_gconst_fanout mode cst_id fanout_acc in
    (fanout_acc, old_gconsts_missing + 1)
  | (Some cst1, Some cst2) ->
    let is_same_signature = Poly.( = ) cst1 cst2 in
    if is_same_signature then
      (fanout_acc, old_gconsts_missing)
    else
      let dep = Dep.GConst cst_id in
      let fanout_acc = Fanout.add_fanout_of mode dep fanout_acc in
      (fanout_acc, old_gconsts_missing)

let get_gconsts_deps ~ctx old_gconsts gconsts =
  let mode = Provider_context.get_deps_mode ctx in
  let { VersionedSSet.removed; kept; added } = gconsts in
  let fanout =
    Fanout.empty
    |> SSet.fold (add_gconst_fanout mode) removed
    |> SSet.fold (add_gconst_fanout mode) added
  in
  SSet.fold (get_gconst_deps ~ctx ~mode old_gconsts) kept (fanout, 0)

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

let get_module_deps ~ctx ~mode old_modules mid (fanout_acc, old_modules_missing)
    : Fanout.t * int =
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
    let fanout_acc = add_module_fanout mode mid fanout_acc in
    (fanout_acc, old_modules_missing + 1)
  | (Some module1, Some module2) ->
    if
      rules_changed module1.mdt_exports module2.mdt_exports
      || rules_changed module1.mdt_imports module2.mdt_imports
    then
      let fanout_acc = add_module_fanout mode mid fanout_acc in
      (fanout_acc, old_modules_missing)
    else
      (fanout_acc, old_modules_missing)

let get_modules_deps ~ctx ~old_modules ~modules =
  let mode = Provider_context.get_deps_mode ctx in
  let { VersionedSSet.removed; kept; added } = modules in
  let fanout =
    Fanout.empty
    |> SSet.fold (add_module_fanout mode) removed
    |> SSet.fold (add_module_fanout mode) added
  in
  SSet.fold (get_module_deps ~ctx ~mode old_modules) kept (fanout, 0)
