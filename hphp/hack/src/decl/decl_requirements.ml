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
let flatten_parent_class_reqs
    env
    class_cache
    shallow_class
    (req_ancestors, req_ancestors_extends)
    parent_ty =
  let (_, (parent_pos, parent_name), parent_params) =
    Decl_utils.unwrap_class_type parent_ty
  in
  let parent_type =
    Decl_env.get_class_add_dep env parent_name ~cache:class_cache
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
    | Ast_defs.Cnormal
    | Ast_defs.Cabstract ->
      (* not necessary to accumulate req_ancestors_extends for classes --
       * it's not used *)
      (req_ancestors, SSet.empty)
    | Ast_defs.Ctrait
    | Ast_defs.Cinterface ->
      let req_ancestors_extends =
        SSet.union parent_type.dc_req_ancestors_extends req_ancestors_extends
      in
      (req_ancestors, req_ancestors_extends)
    | Ast_defs.Cenum -> assert false)

let declared_class_req env class_cache (requirements, req_extends) req_ty =
  let (_, (req_pos, req_name), _) = Decl_utils.unwrap_class_type req_ty in
  let req_type = Decl_env.get_class_add_dep env req_name ~cache:class_cache in
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
  let h = Caml.Hashtbl.create 0 in
  List.rev_filter_map req_extends ~f:(fun (parent_pos, ty) ->
      match get_node ty with
      | Tapply (name, hl) ->
        let hl = List.map hl ~f:Decl_pos_utils.NormalizeSig.ty in
        begin
          try
            let hl' = Caml.Hashtbl.find h name in
            if not (List.equal equal_decl_ty hl hl') then
              raise Exit
            else
              None
          with
          | Exit
          | Caml.Not_found ->
            Caml.Hashtbl.add h name hl;
            Some (parent_pos, ty)
        end
      | _ -> Some (parent_pos, ty))

let get_class_requirements env class_cache shallow_class =
  let req_ancestors_extends = SSet.empty in
  let acc = ([], req_ancestors_extends) in
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
      ( if Ast_defs.(equal_class_kind shallow_class.sc_kind Cinterface) then
        shallow_class.sc_extends
      else
        shallow_class.sc_implements )
  in
  let (req_extends, req_ancestors_extends) = acc in
  let req_extends = naive_dedup req_extends in
  (req_extends, req_ancestors_extends)
