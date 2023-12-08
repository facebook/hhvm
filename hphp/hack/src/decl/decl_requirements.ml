(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Decl_defs
open Shallow_decl_defs
open Typing_defs
module Inst = Decl_instantiate

let make_substitution class_type class_parameters =
  Inst.make_subst class_type.dc_tparams class_parameters

(* Accumulate requirements so that we can successfully check the bodies
 * of trait methods / check that classes satisfy these requirements *)
let flatten_parent_class_reqs env class_cache shallow_class acc parent_ty =
  let (req_ancestors, req_ancestors_extends) = acc in
  let (_, (parent_pos, parent_name), parent_params) =
    Decl_utils.unwrap_class_type parent_ty
  in
  let parent_type =
    Decl_env.get_class_and_add_dep
      ~cache:class_cache
      ~shmem_fallback:false
      ~fallback:Decl_env.no_fallback
      env
      parent_name
  in
  match parent_type with
  | None ->
    (* The class lives in PHP *)
    (req_ancestors, req_ancestors_extends)
  | Some parent_type ->
    let subst = make_substitution parent_type parent_params in
    let req_ancestors =
      List.rev_map_append
        parent_type.dc_req_ancestors
        req_ancestors
        ~f:(fun (_p, ty) ->
          let ty = Inst.instantiate subst ty in
          (parent_pos, ty))
    in
    (match shallow_class.sc_kind with
    | Ast_defs.Cclass _ ->
      (* not necessary to accumulate req_ancestors_extends for classes --
       * it's not used *)
      (req_ancestors, SSet.empty)
    | Ast_defs.Ctrait
    | Ast_defs.Cinterface ->
      let req_ancestors_extends =
        SSet.union parent_type.dc_req_ancestors_extends req_ancestors_extends
      in
      (req_ancestors, req_ancestors_extends)
    | Ast_defs.Cenum
    | Ast_defs.Cenum_class _ ->
      assert false)

let declared_class_req env class_cache acc req_ty =
  let (requirements, req_extends) = acc in
  let (_, (req_pos, req_name), _) = Decl_utils.unwrap_class_type req_ty in
  let req_type =
    Decl_env.get_class_and_add_dep
      ~cache:class_cache
      ~shmem_fallback:false
      ~fallback:Decl_env.no_fallback
      env
      req_name
  in
  let req_extends = SSet.add req_name req_extends in
  (* since the req is declared on this class, we should
   * emphatically *not* substitute: a require extends Foo<T> is
   * going to be this class's <T> *)
  let requirements = (req_pos, req_ty) :: requirements in
  match req_type with
  | None ->
    (* The class lives in PHP : error?? *)
    (requirements, req_extends)
  | Some parent_type ->
    (* The parent class lives in Hack *)
    let req_extends = SSet.union parent_type.dc_extends req_extends in
    let req_extends = SSet.union parent_type.dc_xhp_attr_deps req_extends in
    (* the req may be of an interface that has reqs of its own; the
     * flattened ancestry required by *those* reqs need to be added
     * in to, e.g., interpret accesses to protected functions inside
     * traits *)
    let req_extends =
      SSet.union parent_type.dc_req_ancestors_extends req_extends
    in
    (requirements, req_extends)

let declared_class_req_classes required_classes req_ty =
  let (_, (req_pos, _), _) = Decl_utils.unwrap_class_type req_ty in
  (req_pos, req_ty) :: required_classes

let flatten_parent_class_class_reqs
    env class_cache req_classes_ancestors parent_ty =
  let (_, (parent_pos, parent_name), parent_params) =
    Decl_utils.unwrap_class_type parent_ty
  in
  let parent_type =
    Decl_env.get_class_and_add_dep
      ~cache:class_cache
      ~shmem_fallback:false
      ~fallback:Decl_env.no_fallback
      env
      parent_name
  in
  match parent_type with
  | None ->
    (* The class lives in PHP *)
    req_classes_ancestors
  | Some parent_type ->
    let subst = make_substitution parent_type parent_params in
    List.rev_map_append
      parent_type.dc_req_class_ancestors
      req_classes_ancestors
      ~f:(fun (_p, ty) ->
        let ty = Inst.instantiate subst ty in
        (parent_pos, ty))

(* Cheap hack: we cannot do unification / subtyping in the decl phase because
 * the type arguments of the types that we are trying to unify may not have
 * been declared yet. See the test iface_require_circular.php for details.
 *
 * However, we don't want a super long req_extends list because of the perf
 * overhead. And while we can't do proper unification we can dedup types
 * that are syntactically equivalent.
 *
 * A nicer solution might be to add a phase in between type-decl and type-check
 * that prunes the list via proper subtyping, but that's a little more work
 * than I'm willing to do now. *)
let naive_dedup req_extends =
  (* maps class names to type params *)
  let h = String.Table.create () in
  List.rev_filter_map req_extends ~f:(fun (parent_pos, ty) ->
      match get_node ty with
      | Tapply ((_, name), hl) ->
        let hl = List.map hl ~f:Decl_pos_utils.NormalizeSig.ty in
        begin
          try
            let hl' = Hashtbl.find_exn h name in
            if not (List.equal equal_decl_ty hl hl') then
              raise Exit
            else
              None
          with
          | Exit
          | Stdlib.Not_found
          | Not_found_s _ ->
            Hashtbl.set h ~key:name ~data:hl;
            Some (parent_pos, ty)
        end
      | _ -> Some (parent_pos, ty))

let get_class_requirements env class_cache shallow_class =
  let acc = ([], SSet.empty) in
  let acc =
    List.fold_left
      ~f:(declared_class_req env class_cache)
      ~init:acc
      shallow_class.sc_req_extends
  in
  let acc =
    List.fold_left
      ~f:(declared_class_req env class_cache)
      ~init:acc
      shallow_class.sc_req_implements
  in
  let acc =
    List.fold_left
      ~f:(flatten_parent_class_reqs env class_cache shallow_class)
      ~init:acc
      shallow_class.sc_uses
  in
  let acc =
    List.fold_left
      ~f:(flatten_parent_class_reqs env class_cache shallow_class)
      ~init:acc
      (if Ast_defs.is_c_interface shallow_class.sc_kind then
        shallow_class.sc_extends
      else
        shallow_class.sc_implements)
  in
  let (req_extends, req_ancestors_extends) = acc in
  let req_extends = naive_dedup req_extends in

  let req_classes =
    List.fold_left
      ~f:declared_class_req_classes
      ~init:[]
      shallow_class.sc_req_class
  in
  let req_classes =
    List.fold_left
      ~f:(flatten_parent_class_class_reqs env class_cache)
      ~init:req_classes
      shallow_class.sc_uses
  in
  let req_classes = naive_dedup req_classes in

  (req_extends, req_ancestors_extends, req_classes)
