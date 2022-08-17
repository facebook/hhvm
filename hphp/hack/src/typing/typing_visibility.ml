(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 * *)
open Hh_prelude
open Aast
open Typing_defs
open Utils
module Env = Typing_env
module TUtils = Typing_utils
module Cls = Decl_provider.Class
module Module = Typing_modules

(* Is a private member defined on class/trait [origin_id] visible
 * from code in class/trait [self_id]?
 *
 * For almost all circumstances, this is just a case of checking
 * that they are the same, *except* for the special case of
 * where-constraints on the this type defined on a trait, for example:
 *
 * class C { private static function foo():void { } }
 * trait TR where this = C {
 *   ...C::foo...
 * }
 *)
let is_private_visible env origin_id self_id =
  if String.equal origin_id self_id then
    None
  else
    let in_bounds bounds =
      List.exists bounds ~f:(fun t ->
          match get_node t with
          | Tapply ((_, name), _) -> String.equal origin_id name
          | _ -> false)
    in
    match Env.get_class env self_id with
    | Some cls
      when Ast_defs.is_c_trait (Cls.kind cls)
           && in_bounds (Cls.upper_bounds_on_this_from_constraints cls)
           && in_bounds (Cls.lower_bounds_on_this_from_constraints cls) ->
      None
    | _ -> Some "You cannot access this member"

let is_protected_visible env origin_id self_id =
  if TUtils.has_ancestor_including_req_refl env self_id origin_id then
    None
  else
    match Env.get_class env origin_id with
    | None -> None
    | Some origin_class ->
      (* Parents can call direct children's protected methods
       * (like a constructor)
       *)
      if Cls.has_ancestor origin_class self_id then
        None
      else
        Some
          ("Cannot access this protected member, you don't extend "
          ^ strip_ns origin_id)

let is_private_visible_for_class env x self_id cid class_ =
  match cid with
  | CIstatic ->
    let my_class = Env.get_class env self_id in
    (match my_class with
    | Some cls when Cls.final cls -> None
    | _ ->
      Some
        "Private members cannot be accessed with static:: since a child class may also have an identically named private member")
  | CIparent -> Some "You cannot access a private member with parent::"
  | CIself -> is_private_visible env x self_id
  | CI (_, called_ci) ->
    (match is_private_visible env x self_id with
    | None -> None
    | Some _ ->
      begin
        match Env.get_class env called_ci with
        | Some cls when Ast_defs.is_c_trait (Cls.kind cls) ->
          Some
            "You cannot access private members using the trait's name (did you mean to use self::?)"
        | _ -> Some "You cannot access this member"
      end)
  | CIexpr _ ->
    if Cls.final class_ then
      None
    else
      Some
        "Private members cannot be accessed dynamically. Did you mean to use 'self::'?"

let is_internal_visible env target =
  match
    Module.can_access
      ~env
      ~current:(Env.get_current_module env)
      ~target:(Some target)
  with
  | `Yes -> None
  | `Disjoint (current, target) ->
    Some
      (Printf.sprintf
         "You cannot access internal members from module `%s` in module `%s`"
         target
         current)
  | `Outside _ -> Some "You cannot access internal members outside of a module"
  | `OutsideViaTrait _ ->
    Some "You cannot access internal members inside a public trait"

let check_internal_access ~use_pos ~in_signature ~def_pos env internal module_ =
  let module_err_opt =
    if internal then
      let open Typing_error.Primary.Modules in
      match
        Module.can_access
          ~env
          ~current:(Env.get_current_module env)
          ~target:module_
      with
      | `Yes when in_signature && not (Env.get_internal env) ->
        Some (Module_hint { pos = use_pos; decl_pos = def_pos })
      | `Yes -> None
      | `Disjoint (current, target) ->
        Some
          (Module_mismatch
             {
               pos = use_pos;
               decl_pos = def_pos;
               current_module_opt = Some current;
               target_module = target;
             })
      | `Outside target ->
        Some
          (Module_mismatch
             {
               pos = use_pos;
               decl_pos = def_pos;
               current_module_opt = None;
               target_module = target;
             })
      | `OutsideViaTrait trait_pos ->
        Some (Module_unsafe_trait_access { access_pos = use_pos; trait_pos })
    else
      None
  in
  Option.map ~f:Typing_error.modules module_err_opt

let check_classname_access ~use_pos ~in_signature env cls =
  check_internal_access
    ~use_pos
    ~in_signature
    ~def_pos:(Cls.pos cls)
    env
    (Cls.internal cls)
    (Cls.get_module cls)

let check_typedef_access ~use_pos ~in_signature env td =
  check_internal_access
    ~use_pos
    ~in_signature
    ~def_pos:td.td_pos
    env
    td.td_internal
    (Option.map td.td_module ~f:snd)

let is_visible_for_obj ~is_method env vis =
  let member_ty =
    if is_method then
      "method"
    else
      "property"
  in
  match vis with
  | Vpublic -> None
  | Vprivate x ->
    (match Env.get_self_id env with
    | None -> Some ("You cannot access this " ^ member_ty)
    | Some self_id -> is_private_visible env x self_id)
  | Vprotected x ->
    (match Env.get_self_id env with
    | None -> Some ("You cannot access this " ^ member_ty)
    | Some self_id -> is_protected_visible env x self_id)
  | Vinternal m -> is_internal_visible env m

(* The only permitted way to access an LSB property is via
   static::, ClassName::, or $class_name:: *)
let is_lsb_permitted cid =
  match cid with
  | CIself -> Some "__LSB properties cannot be accessed with self::"
  | CIparent -> Some "__LSB properties cannot be accessed with parent::"
  | _ -> None

(* LSB property accessibility is relative to the defining class *)
let is_lsb_accessible env vis =
  match vis with
  | Vpublic -> None
  | Vprivate x ->
    (match Env.get_self_id env with
    | None -> Some "You cannot access this property"
    | Some self_id -> is_private_visible env x self_id)
  | Vprotected x ->
    (match Env.get_self_id env with
    | None -> Some "You cannot access this property"
    | Some self_id -> is_protected_visible env x self_id)
  | Vinternal m -> is_internal_visible env m

let is_lsb_visible_for_class env vis cid =
  match is_lsb_permitted cid with
  | Some x -> Some x
  | None -> is_lsb_accessible env vis

let is_visible_for_class ~is_method env (vis, lsb) cid cty =
  if lsb then
    is_lsb_visible_for_class env vis cid
  else
    let member_ty =
      if is_method then
        "method"
      else
        "property"
    in
    match vis with
    | Vpublic -> None
    | Vprivate x ->
      (match Env.get_self_id env with
      | None -> Some ("You cannot access this " ^ member_ty)
      | Some self_id -> is_private_visible_for_class env x self_id cid cty)
    | Vprotected x ->
      (match Env.get_self_id env with
      | None -> Some ("You cannot access this " ^ member_ty)
      | Some self_id ->
        let their_class = Env.get_class env x in
        (match (cid, their_class) with
        | (CI _, Some cls) when Ast_defs.is_c_trait (Cls.kind cls) ->
          Some
            "You cannot access protected members using the trait's name (did you mean to use static:: or self::?)"
        | _ -> is_protected_visible env x self_id))
    | Vinternal m -> is_internal_visible env m

let is_visible ~is_method env (vis, lsb) cid class_ =
  let msg_opt =
    match cid with
    | Some cid -> is_visible_for_class ~is_method env (vis, lsb) cid class_
    | None -> is_visible_for_obj ~is_method env vis
  in
  Option.is_none msg_opt

let visibility_error p msg (p_vis, vis) =
  let s = Typing_defs.string_of_visibility vis in
  let msg_vis = "This member is " ^ s in
  Typing_error.(
    primary
    @@ Primary.Visibility
         { pos = p; msg; decl_pos = p_vis; reason_msg = msg_vis })

let check_obj_access ~is_method ~use_pos ~def_pos env vis =
  Option.map (is_visible_for_obj ~is_method env vis) ~f:(fun msg ->
      visibility_error use_pos msg (def_pos, vis))

let check_expression_tree_vis ~use_pos ~def_pos env vis =
  let open Typing_error in
  if Typing_env.is_in_expr_tree env then
    match vis with
    | Vpublic -> None
    | _ ->
      Some
        (expr_tree
        @@ Primary.Expr_tree.Expression_tree_non_public_member
             { pos = use_pos; decl_pos = def_pos })
  else
    None

let check_inst_meth_access ~use_pos ~def_pos vis =
  let open Typing_error in
  match vis with
  | Vprivate _ ->
    Some
      (primary
      @@ Primary.Private_inst_meth { decl_pos = def_pos; pos = use_pos })
  | Vprotected _ ->
    Some
      (primary
      @@ Primary.Protected_inst_meth { decl_pos = def_pos; pos = use_pos })
  | _ -> None

let check_meth_caller_access ~use_pos ~def_pos vis =
  let open Typing_error in
  match vis with
  | Vprivate _ ->
    Some
      (primary
      @@ Primary.Private_meth_caller { decl_pos = def_pos; pos = use_pos })
  | Vprotected _ ->
    Some
      (primary
      @@ Primary.Protected_meth_caller { decl_pos = def_pos; pos = use_pos })
  | _ -> None

let check_class_access ~is_method ~use_pos ~def_pos env (vis, lsb) cid class_ =
  Option.map
    (is_visible_for_class ~is_method env (vis, lsb) cid class_)
    ~f:(fun msg -> visibility_error use_pos msg (def_pos, vis))

let check_deprecated ~use_pos ~def_pos env deprecated =
  if Typing_env_types.(env.in_support_dynamic_type_method_check) then
    None
  else
    Option.map deprecated ~f:(fun s ->
        Typing_error.(
          primary
          @@ Primary.Deprecated_use
               { pos = use_pos; decl_pos_opt = Some def_pos; msg = s }))
