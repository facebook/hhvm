(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Decl_defs
open Shallow_decl_defs
open Typing_defs

module Inst = Decl_instantiate

let check_arity pos class_name class_type class_parameters =
  let arity = List.length class_type.dc_tparams in
  if List.length class_parameters <> arity
  then Errors.class_arity pos class_type.dc_pos class_name arity;
  ()

let make_substitution pos class_name class_type class_parameters =
  check_arity pos class_name class_type class_parameters;
  Inst.make_subst class_type.dc_tparams class_parameters

(* Accumulate requirements so that we can successfully check the bodies
 * of trait methods / check that classes satisfy these requirements *)
let flatten_parent_class_reqs env shallow_class
    (req_ancestors, req_ancestors_extends) parent_ty =
  let _, (parent_pos, parent_name), parent_params =
    Decl_utils.unwrap_class_type parent_ty in
  let parent_type = Decl_env.get_class_dep env parent_name in

  match parent_type with
  | None ->
    (* The class lives in PHP *)
    req_ancestors, req_ancestors_extends
  | Some parent_type ->
    let subst =
      make_substitution parent_pos parent_name parent_type parent_params in
    let req_ancestors =
      List.rev_map_append parent_type.dc_req_ancestors req_ancestors
        begin fun (_p, ty) ->
          let ty = Inst.instantiate subst ty in
          parent_pos, ty
        end in
    match shallow_class.sc_kind with
    | Ast.Cnormal | Ast.Cabstract ->
      (* not necessary to accumulate req_ancestors_extends for classes --
       * it's not used *)
      req_ancestors, SSet.empty
    | Ast.Ctrait | Ast.Cinterface ->
      let req_ancestors_extends = SSet.union
        parent_type.dc_req_ancestors_extends req_ancestors_extends in
      req_ancestors, req_ancestors_extends
    | Ast.Cenum | Ast.Crecord -> assert false

let declared_class_req env (requirements, req_extends) req_ty =
  let _, (req_pos, req_name), _ = Decl_utils.unwrap_class_type req_ty in
  let req_type = Decl_env.get_class_dep env req_name in
  let req_extends = SSet.add req_name req_extends in
  (* since the req is declared on this class, we should
   * emphatically *not* substitute: a require extends Foo<T> is
   * going to be this class's <T> *)
  let requirements = (req_pos, req_ty) :: requirements in
  match req_type with
  | None -> (* The class lives in PHP : error?? *)
    requirements, req_extends
  | Some parent_type -> (* The parent class lives in Hack *)
    let req_extends = SSet.union parent_type.dc_extends req_extends in
    let req_extends = SSet.union parent_type.dc_xhp_attr_deps req_extends in
    (* the req may be of an interface that has reqs of its own; the
     * flattened ancestry required by *those* reqs need to be added
     * in to, e.g., interpret accesses to protected functions inside
     * traits *)
    let req_extends =
      SSet.union parent_type.dc_req_ancestors_extends req_extends in
    requirements, req_extends

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
  List.rev_filter_map req_extends begin fun (parent_pos, ty) ->
    match ty with
    | _r, Tapply (name, hl) ->
      let hl = List.map hl Decl_pos_utils.NormalizeSig.ty in
      begin try
        let hl' = Caml.Hashtbl.find h name in
        if List.compare Pervasives.compare hl hl' <> 0 then
          raise Exit
        else
          None
      with
      | Exit
      | Caml.Not_found ->
        Caml.Hashtbl.add h name hl;
        Some (parent_pos, ty)
      end
    | _ -> Some (parent_pos, ty)
  end

let get_class_requirements env shallow_class =
  let req_ancestors_extends = SSet.empty in
  let acc = ([], req_ancestors_extends) in
  let acc =
    List.fold_left ~f:(declared_class_req env)
      ~init:acc shallow_class.sc_req_extends in
  let acc =
    List.fold_left ~f:(declared_class_req env)
      ~init:acc shallow_class.sc_req_implements in
  let acc =
    List.fold_left ~f:(flatten_parent_class_reqs env shallow_class)
      ~init:acc shallow_class.sc_uses in
  let acc =
    List.fold_left ~f:(flatten_parent_class_reqs env shallow_class)
      ~init:acc (if shallow_class.sc_kind = Ast.Cinterface then
          shallow_class.sc_extends else shallow_class.sc_implements) in
  let req_extends, req_ancestors_extends = acc in
  let req_extends = naive_dedup req_extends in
  req_extends, req_ancestors_extends
