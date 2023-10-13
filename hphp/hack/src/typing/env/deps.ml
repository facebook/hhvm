(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Cls = Decl_provider.Class
module Dep = Typing_deps.Dep

let get_current_decl env =
  Option.map
    env.Typing_env_types.decl_env.Decl_env.droot
    ~f:Dep.to_decl_reference

let add_fine_dep_if_enabled env dependency =
  let open Option.Let_syntax in
  let decl_env = env.Typing_env_types.decl_env in
  let ctx = decl_env.Decl_env.ctx in
  (* We resolve the dependency to its origin. See
   * [Typing_extends.add_pessimisation_dependency] for details. *)
  let dependency_on_origin () =
    match dependency with
    | Dep.Method (class_name, method_name) ->
      let origin_name =
        let* cls = Decl_provider.get_class ctx class_name in
        let* elt = Cls.get_method cls method_name in
        Some elt.Typing_defs.ce_origin
      in
      let origin_name = Option.value origin_name ~default:class_name in
      Dep.Method (origin_name, method_name)
    | Dep.SMethod (class_name, method_name) ->
      let origin_name =
        let* cls = Decl_provider.get_class ctx class_name in
        let* elt = Cls.get_smethod cls method_name in
        Some elt.Typing_defs.ce_origin
      in
      let origin_name = Option.value origin_name ~default:class_name in
      Dep.SMethod (origin_name, method_name)
    | _ -> dependency
  in
  if
    TypecheckerOptions.record_fine_grained_dependencies
    @@ Typing_env_types.(env.genv.tcopt)
  then
    let dependency = dependency_on_origin () in
    Typing_pessimisation_deps.try_add_fine_dep
      (Provider_context.get_deps_mode ctx)
      decl_env.Decl_env.droot
      decl_env.Decl_env.droot_member
      dependency

let mark_declared_fine mode dep =
  Typing_pessimisation_deps.try_add_fine_dep mode (Some Dep.Declares) None dep

let add_dependency_edge (env : Typing_env_types.env) dep : unit =
  let decl_env = env.Typing_env_types.decl_env in
  Option.iter decl_env.Decl_env.droot ~f:(fun root ->
      let ctx = decl_env.Decl_env.ctx in
      let deps_mode = Provider_context.get_deps_mode ctx in
      Typing_deps.add_idep deps_mode root dep);
  add_fine_dep_if_enabled env dep

let mark_declared env dep =
  let mode =
    Provider_context.get_deps_mode env.Typing_env_types.decl_env.Decl_env.ctx
  in
  Typing_deps.add_idep mode Dep.Declares dep;
  mark_declared_fine mode dep;
  ()

let mark_class_constant_declared env class_name const_name =
  mark_declared env (Dep.Const (class_name, const_name))

let mark_typeconst_declared env class_name typeconst_name =
  mark_declared env (Dep.Const (class_name, typeconst_name))

let mark_property_declared env ~is_static class_name property_name =
  mark_declared
    env
    (if is_static then
      Dep.SProp (class_name, property_name)
    else
      Dep.Prop (class_name, property_name))

let mark_method_declared env ~is_static class_name method_name =
  mark_declared
    env
    (if is_static then
      Dep.SMethod (class_name, method_name)
    else
      Dep.Method (class_name, method_name))

let mark_constructor_declared env class_name =
  mark_declared env (Dep.Constructor class_name)

let mark_xhp_attribute_declared env class_name attr_name =
  mark_declared env (Dep.Prop (class_name, attr_name))

let make_depend_on_gconst env name def =
  match def with
  | Some cst when Pos_or_decl.is_hhi cst.Typing_defs.cd_pos -> ()
  | _ -> add_dependency_edge env (Dep.GConst name)

let make_depend_on_fun env name fd =
  match fd with
  | Some fd when Pos_or_decl.is_hhi fd.Typing_defs.fe_pos -> ()
  | _ -> add_dependency_edge env (Dep.Fun name)

let make_depend_on_class_name env class_name =
  add_dependency_edge env (Dep.Type class_name)

let make_depend_on_class env x cd =
  match cd with
  | Some cd when Pos_or_decl.is_hhi (Cls.pos cd) -> ()
  | _ -> make_depend_on_class_name env x

let make_depend_on_constructor_name env class_name =
  make_depend_on_class_name env class_name;
  add_dependency_edge env (Dep.Constructor class_name);
  ()

let make_depend_on_constructor env class_ =
  if not (Pos_or_decl.is_hhi (Cls.pos class_)) then (
    make_depend_on_constructor_name env (Cls.name class_);
    if
      not
        (TypecheckerOptions.optimized_member_fanout
           Typing_env_types.(env.genv.tcopt))
    then
      Option.iter
        (fst (Cls.construct class_))
        ~f:(fun ce ->
          make_depend_on_constructor_name env ce.Typing_defs.ce_origin)
  )

let make_depend_on_module_name env module_name =
  add_dependency_edge env (Dep.Module module_name)

let make_depend_on_module env name md =
  match md with
  | Some md when Pos_or_decl.is_hhi md.Typing_defs.mdt_pos -> ()
  | _ -> make_depend_on_module_name env name

let make_depend_on_parent env ~skip_constructor_dep ~is_req name class_ =
  match class_ with
  | Some cd when Pos_or_decl.is_hhi (Cls.pos cd) -> ()
  | _ ->
    if not skip_constructor_dep then make_depend_on_constructor_name env name;
    let dep =
      if
        TypecheckerOptions.optimized_member_fanout
          Typing_env_types.(env.genv.tcopt)
        && is_req
      then
        Dep.RequireExtends name
      else
        Dep.Extends name
    in
    add_dependency_edge env dep

let add_member_dep ~is_method ~is_static env (class_ : Cls.t) mid class_elt_opt
    =
  let add_dep cid =
    let dep =
      match (is_method, is_static) with
      | (true, true) -> Dep.SMethod (cid, mid)
      | (true, false) -> Dep.Method (cid, mid)
      | (false, true) -> Dep.SProp (cid, mid)
      | (false, false) -> Dep.Prop (cid, mid)
    in
    add_dependency_edge env dep
  in
  if not (Pos_or_decl.is_hhi (Cls.pos class_)) then (
    make_depend_on_class_name env (Cls.name class_);
    add_dep (Cls.name class_);
    if
      not
        (TypecheckerOptions.optimized_member_fanout
           Typing_env_types.(env.genv.tcopt))
    then
      Option.iter class_elt_opt ~f:(fun ce -> add_dep ce.Typing_defs.ce_origin)
  );
  ()

let make_depend_on_class_const env class_ name =
  if not (Pos_or_decl.is_hhi (Cls.pos class_)) then (
    make_depend_on_class_name env (Cls.name class_);
    add_dependency_edge env (Dep.Const (Cls.name class_, name))
  )

let make_depend_on_typedef env name td =
  match td with
  | Some td when Pos_or_decl.is_hhi td.Typing_defs.td_pos -> ()
  | _ -> make_depend_on_class_name env name
