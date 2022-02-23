(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Checks that a class implements an interface *)
(*****************************************************************************)

open Hh_prelude
open Option.Monad_infix
open Typing_defs
module Env = Typing_env
module Dep = Typing_deps.Dep
module TUtils = Typing_utils
module Inst = Decl_instantiate
module Phase = Typing_phase
module SN = Naming_special_names
module Cls = Decl_provider.Class
module MakeType = Typing_make_type
module TCO = TypecheckerOptions

module MemberKind = struct
  type t =
    | Property
    | Static_property
    | Method
    | Static_method
    | Constructor of { is_consistent: bool }
  [@@deriving eq, ord]

  let is_method member_kind =
    match member_kind with
    | Method
    | Static_method ->
      true
    | Property
    | Static_property
    | Constructor _ ->
      false

  let is_functional member_kind =
    match member_kind with
    | Method
    | Static_method
    | Constructor _ ->
      true
    | Property
    | Static_property ->
      false

  let is_constructor = function
    | Constructor _ -> true
    | _ -> false
end

module MemberKindMap = WrappedMap.Make (MemberKind)

(* This is used to merge members from all parents of a class.
 * Certain class hierarchies are heavy in diamond patterns so merging members avoids doing the
 * same member subtyping multiple times. *)
module ClassEltWParent = struct
  type parent = Pos_or_decl.t * Cls.t

  type t = class_elt * parent

  (* Class elements with the same names and origins should be equal
     modulo type instantiations, which is why we also need to compare types.
     For example,

        interface I<T> {
          public function foo():T;
        }
        interface I1 extends I<string> {}
        interface I2 extends I<int> {}
        class C implements I1, I2 {
          public function foo():int { return 3; }
        }

     When unioning members of I1 and I2, we have two `foo` members with the same
     origin (`I`) but with different types. *)
  let compare : t -> t -> int =
   fun (elt1, _) (elt2, _) ->
    let {
      ce_visibility = _;
      ce_type = type1;
      ce_origin = origin1;
      ce_deprecated = _;
      ce_pos = _;
      ce_flags = _;
    } =
      elt1
    in
    let {
      ce_visibility = _;
      ce_type = type2;
      ce_origin = origin2;
      ce_deprecated = _;
      ce_pos = _;
      ce_flags = _;
    } =
      elt2
    in
    match String.compare origin1 origin2 with
    | 0 -> compare_decl_ty (Lazy.force type1) (Lazy.force type2)
    | x -> x
end

module ClassEltWParentSet = Caml.Set.Make (ClassEltWParent)

let constructor_is_consistent kind =
  match kind with
  | ConsistentConstruct
  | FinalClass ->
    true
  | Inconsistent -> false

(*****************************************************************************)
(* Given a map of members, check that the overriding is correct.
 * Please note that 'members' has a very general meaning here.
 * It can be class variables, methods, static methods etc ... The same logic
 * is applied to verify that the overriding is correct.
 *)
(*****************************************************************************)

(* Rules for visibility *)
let check_visibility parent_vis c_vis parent_pos pos on_error =
  match (parent_vis, c_vis) with
  | (Vprivate _, _) ->
    (* The only time this case should come into play is when the
     * parent_class_elt comes from a trait *)
    ()
  | (Vpublic, Vpublic)
  | (Vprotected _, Vprotected _)
  | (Vprotected _, Vpublic)
  | (Vinternal _, Vpublic) ->
    ()
  | (Vinternal parent_module, (Vprotected _ | Vprivate _)) ->
    let err =
      Typing_error.Secondary.Visibility_override_internal
        { pos; module_name = None; parent_pos; parent_module }
    in
    Errors.add_typing_error @@ Typing_error.(apply_reasons ~on_error err)
  | (Vinternal parent_m, Vinternal child_m) ->
    let err_opt =
      match
        Typing_modules.can_access
          ~current:(Some child_m)
          ~target:(Some parent_m)
      with
      | `Yes -> None
      | `Disjoint (current, target) ->
        Some
          (Typing_error.Secondary.Visibility_override_internal
             {
               pos;
               module_name = Some current;
               parent_pos;
               parent_module = target;
             })
      | `Outside target ->
        Some
          (Typing_error.Secondary.Visibility_override_internal
             { pos; module_name = None; parent_pos; parent_module = target })
    in
    Option.iter err_opt ~f:(fun err ->
        Errors.add_typing_error @@ Typing_error.(apply_reasons ~on_error err))
  | _ ->
    let parent_vis = Typing_defs.string_of_visibility parent_vis in
    let vis = Typing_defs.string_of_visibility c_vis in
    let err =
      Typing_error.Secondary.Visibility_extends
        { pos; vis; parent_pos; parent_vis }
    in
    Errors.add_typing_error @@ Typing_error.(apply_reasons ~on_error err)

let check_class_elt_visibility parent_class_elt class_elt on_error =
  let parent_vis = parent_class_elt.ce_visibility in
  let c_vis = class_elt.ce_visibility in
  let (lazy parent_pos) = parent_class_elt.ce_pos in
  let (lazy pos) = class_elt.ce_pos in
  check_visibility parent_vis c_vis parent_pos pos on_error

let stub_meth_quickfix
    (class_name : string)
    (parent_name : string)
    (meth_name : string)
    (meth : class_elt) : Quickfix.t =
  let title =
    Printf.sprintf
      "Add stub method %s"
      (Markdown_lite.md_codify (Utils.strip_ns parent_name ^ "::" ^ meth_name))
  in
  let new_text = Typing_skeleton.of_method meth_name meth in
  Quickfix.make_classish ~title ~new_text ~classish_name:class_name

let get_member member_kind class_ =
  match member_kind with
  | MemberKind.Property -> Cls.get_prop class_
  | MemberKind.Static_property -> Cls.get_sprop class_
  | MemberKind.Method -> Cls.get_method class_
  | MemberKind.Static_method -> Cls.get_smethod class_
  | MemberKind.Constructor _ -> (fun _ -> fst (Cls.construct class_))

let member_not_implemented_error
    class_name parent_name member_name class_elt pos parent_pos =
  let (lazy defn_pos) = class_elt.ce_pos in
  let quickfixes =
    [stub_meth_quickfix class_name parent_name member_name class_elt]
  in
  let err =
    Typing_error.(
      primary
      @@ Primary.Member_not_implemented
           { member_name; parent_pos; pos; decl_pos = defn_pos; quickfixes })
  in
  Errors.add_typing_error err

let check_subtype_methods
    env ~check_return on_error (r_ancestor, ft_ancestor) (r_child, ft_child) ()
    =
  Typing_subtype_method.(
    (* Add deps here when we override *)
    subtype_method_decl
      ~check_return
      env
      r_child
      ft_child
      r_ancestor
      ft_ancestor
      on_error)

(* An abstract member can be declared in multiple ancestors. Sometimes these
 * declarations can be different, but yet compatible depending on which ancestor
 * we inherit the member from. For example:
 *
 * interface I1 { abstract public function foo(): int; }
 * interface I2 { abstract public function foo(): mixed; }
 *
 * abstract class C implements I1, I2 {}
 *
 * I1::foo() is compatible with I2::foo(), but not vice versa. Hack chooses the
 * signature for C::foo() arbitrarily and can report an error if we make a
 * "wrong" choice. We check for this case and emit an extra line in the error
 * instructing the programmer to redeclare the member to remove the ambiguity.
 *
 * Note: We could detect this case and make the correct choice for the user, but
 * this would require invalidating the current entry we have in the typing heap
 * for this class. We cannot make this choice earlier during typing_decl because
 * a class we depend on during the subtyping may not have been declared yet.
 *)
(* TODO(jjwu): get rid of this for type constants too, and we can delete *)
let check_ambiguous_inheritance f parent child pos class_ origin on_error =
  Errors.try_when
    (f parent child)
    ~if_error_and:(fun () ->
      String.( <> ) (Cls.name class_) origin
      && Errors.has_no_errors (f child parent))
    ~then_:(fun error ->
      Errors.ambiguous_inheritance pos (Cls.name class_) origin error on_error)

(** Checks that we're not overriding a final method. *)
let check_override_final_method parent_class_elt class_elt on_error =
  let is_override_of_final_method =
    get_ce_final parent_class_elt
    && String.( <> ) parent_class_elt.ce_origin class_elt.ce_origin
  in
  if is_override_of_final_method && not (get_ce_synthesized class_elt) then
    (* we have a final method being overridden by a user-declared method *)
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    let (lazy pos) = class_elt.ce_pos in
    Errors.add_typing_error
      Typing_error.(
        apply_reasons ~on_error @@ Secondary.Override_final { pos; parent_pos })

(** Checks that methods annotated with __DynamicallyCallable are only overridden with
    dynamically callable method. *)
let check_dynamically_callable member_name parent_class_elt class_elt on_error =
  if
    get_ce_dynamicallycallable parent_class_elt
    && not (get_ce_dynamicallycallable class_elt)
  then
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    let (lazy pos) = class_elt.ce_pos in
    let (snd_err1, snd_err2) =
      Typing_error.Secondary.
        ( Bad_method_override { pos; member_name },
          Method_not_dynamically_callable { pos; parent_pos } )
    in
    (* Modify the callback so that we append `snd_err2` to `snd_err1` when
       evaluating *)
    let on_error =
      Typing_error.Reasons_callback.prepend_on_apply on_error snd_err1
    in
    Errors.add_typing_error @@ Typing_error.apply_reasons ~on_error snd_err2

(** Check that we are not overriding an __LSB property *)
let check_lsb_overrides
    member_kind member_name parent_class_elt class_elt on_error =
  let parent_is_lsb = get_ce_lsb parent_class_elt in
  if MemberKind.equal MemberKind.Static_property member_kind && parent_is_lsb
  then
    (* __LSB attribute is being overridden *)
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    let (lazy pos) = class_elt.ce_pos in
    Errors.add_typing_error
    @@ Typing_error.(
         apply_reasons ~on_error
         @@ Secondary.Override_lsb { pos; parent_pos; member_name })

(** Check that __LateInit annotation on members are consistent between parents and children. *)
let check_lateinit parent_class_elt class_elt on_error =
  let lateinit_diff =
    Bool.( <> ) (get_ce_lateinit parent_class_elt) (get_ce_lateinit class_elt)
  in
  if lateinit_diff then
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    let (lazy child_pos) = class_elt.ce_pos in
    Errors.add_typing_error
    @@ Typing_error.(
         apply_reasons ~on_error
         @@ Secondary.Bad_lateinit_override
              {
                pos = child_pos;
                parent_pos;
                parent_is_lateinit = get_ce_lateinit parent_class_elt;
              })

let check_xhp_attr_required env parent_class_elt class_elt on_error =
  if not (TypecheckerOptions.check_xhp_attribute (Env.get_tcopt env)) then
    ()
  else
    let is_less_strict = function
      | (Some Xhp_attribute.Required, _)
      | (Some Xhp_attribute.LateInit, Some Xhp_attribute.LateInit)
      | (Some Xhp_attribute.LateInit, None)
      | (None, None) ->
        false
      | (_, _) -> true
    in
    let parent_attr = get_ce_xhp_attr parent_class_elt in
    let attr = get_ce_xhp_attr class_elt in
    match (parent_attr, attr) with
    | ( Some { Xhp_attribute.xa_tag = parent_tag; _ },
        Some { Xhp_attribute.xa_tag = tag; _ } )
      when is_less_strict (tag, parent_tag) ->
      let (lazy parent_pos) = parent_class_elt.ce_pos in
      let (lazy child_pos) = class_elt.ce_pos in
      let lateinit = Markdown_lite.md_codify "@lateinit" in
      let required = Markdown_lite.md_codify "@required" in
      let show = function
        | None -> Printf.sprintf "not %s or %s" required lateinit
        | Some Xhp_attribute.Required -> required
        | Some Xhp_attribute.LateInit -> lateinit
      in
      Errors.add_typing_error
      @@ Typing_error.(
           apply_reasons ~on_error
           @@ Secondary.Bad_xhp_attr_required_override
                {
                  pos = child_pos;
                  tag = show tag;
                  parent_pos;
                  parent_tag = show parent_tag;
                })
    | (_, _) -> ()

let add_member_dep
    env class_ (member_kind, member_name, member_origin, origin_pos) =
  if not (Pos_or_decl.is_hhi origin_pos) then
    let dep =
      match member_kind with
      | MemberKind.Method -> Dep.Method (member_origin, member_name)
      | MemberKind.Static_method -> Dep.SMethod (member_origin, member_name)
      | MemberKind.Static_property -> Dep.SProp (member_origin, member_name)
      | MemberKind.Property -> Dep.Prop (member_origin, member_name)
      | MemberKind.Constructor _ -> Dep.Constructor member_origin
    in
    Typing_deps.add_idep
      (Env.get_deps_mode env)
      (Dep.Type (Cls.name class_))
      dep

let detect_multiple_concrete_defs class_elt class_ parent_class_elt parent_class
    =
  (* We want to check if there are conflicting trait declarations of a class member.
   * If the parent we are checking is a trait and the member's origin both isn't
   * that parent and isn't the class itself, then it must come from another trait
   * and there is a conflict.
   *
   * We rule out cases where any of the traits' member
   * is synthetic (from a requirement) or abstract. *)
  match Cls.kind parent_class with
  | Ast_defs.Ctrait ->
    (not (get_ce_synthesized class_elt))
    && (not (get_ce_abstract class_elt))
    && (not (get_ce_abstract parent_class_elt))
    && String.( <> ) class_elt.ce_origin (Cls.name class_)
  | Ast_defs.(Cinterface | Cclass _ | Cenum | Cenum_class _) -> false

let check_compatible_sound_dynamic_attributes
    env member_name member_kind parent_class_elt class_elt on_error =
  if
    (not (MemberKind.is_constructor member_kind))
    && TypecheckerOptions.enable_sound_dynamic
         (Provider_context.get_tcopt (Env.get_ctx env))
    && get_ce_support_dynamic_type parent_class_elt
    && not (get_ce_support_dynamic_type class_elt)
  then
    let (lazy pos) = class_elt.ce_pos in
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    Errors.add_typing_error
    @@ Typing_error.(
         apply_reasons ~on_error
         @@ Secondary.Override_method_support_dynamic_type
              {
                pos;
                parent_pos;
                parent_origin = parent_class_elt.ce_origin;
                method_name = member_name;
              })

let check_prop_const_mismatch parent_class_elt class_elt on_error =
  if Bool.( <> ) (get_ce_const class_elt) (get_ce_const parent_class_elt) then
    Errors.add_typing_error
      Typing_error.(
        apply_reasons ~on_error
        @@ Secondary.Overriding_prop_const_mismatch
             {
               pos = Lazy.force class_elt.ce_pos;
               is_const = get_ce_const class_elt;
               parent_pos = Lazy.force parent_class_elt.ce_pos;
               parent_is_const = get_ce_const parent_class_elt;
             })

let check_abstract_overrides_concrete env member_kind parent_class_elt class_elt
    =
  if (not (get_ce_abstract parent_class_elt)) && get_ce_abstract class_elt then
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See override_abstract_concrete.php test case for example. *)
    Errors.add_typing_error
      Typing_error.(
        assert_in_current_decl ~ctx:(Env.get_current_decl_and_file env)
        @@ Secondary.Abstract_concrete_override
             {
               pos = Lazy.force class_elt.ce_pos;
               parent_pos = Lazy.force parent_class_elt.ce_pos;
               kind =
                 (if MemberKind.is_functional member_kind then
                   `method_
                 else
                   `property);
             })

let check_multiple_concrete_definitions
    check_member_unique
    class_
    member_name
    member_kind
    parent_class_elt
    class_elt
    on_error =
  if
    check_member_unique
    && (MemberKind.is_functional member_kind || get_ce_const class_elt)
  then
    (* Multiple concrete trait definitions, error *)
    Errors.add_typing_error
      Typing_error.(
        apply_reasons ~on_error
        @@ Secondary.Multiple_concrete_defs
             {
               pos = Lazy.force class_elt.ce_pos;
               parent_pos = Lazy.force parent_class_elt.ce_pos;
               origin = class_elt.ce_origin;
               parent_origin = parent_class_elt.ce_origin;
               name = member_name;
               class_name = Cls.name class_;
             })

(* Check that overriding is correct *)
let check_override
    env
    ~check_member_unique
    member_name
    member_kind
    class_
    parent_class
    parent_class_elt
    class_elt
    on_error =
  add_member_dep
    env
    class_
    (member_kind, member_name, parent_class_elt.ce_origin, Cls.pos parent_class);
  (* If the class element is defined in the class that we're checking, then
   * don't wrap with the extra
   * "Class ... does not correctly implement all required members" message *)
  let on_error =
    if String.equal class_elt.ce_origin (Cls.name class_) then
      Env.unify_error_assert_primary_pos_in_current_decl env
    else
      on_error
  in

  if MemberKind.is_method member_kind then begin
    (* We first verify that we aren't overriding a final method *)
    (* We only check for final overrides on methods, not properties *)
    (* we don't check constructors, as they are already checked
     * in the decl phase *)
    check_override_final_method parent_class_elt class_elt on_error;
    check_dynamically_callable member_name parent_class_elt class_elt on_error
  end;

  (* Verify that we are not overriding an __LSB property *)
  check_lsb_overrides
    member_kind
    member_name
    parent_class_elt
    class_elt
    on_error;
  check_lateinit parent_class_elt class_elt on_error;
  check_xhp_attr_required env parent_class_elt class_elt on_error;
  check_class_elt_visibility parent_class_elt class_elt on_error;
  check_prop_const_mismatch parent_class_elt class_elt on_error;
  check_abstract_overrides_concrete env member_kind parent_class_elt class_elt;

  let (lazy pos) = class_elt.ce_pos in
  let (lazy parent_pos) = parent_class_elt.ce_pos in

  let snd_err =
    let open Typing_error.Secondary in
    if MemberKind.is_functional member_kind then
      Bad_method_override { pos; member_name }
    else
      Bad_prop_override { pos; member_name }
  in
  (* Modify the `Typing_error.Reasons_callback.t` so that we always end up
     with the error code given by `snd_err` and the reasons of whatever
     error it is applied to are appended to the reasons given by `snd_err`
  *)
  let on_error =
    Typing_error.Reasons_callback.prepend_on_apply on_error snd_err
  in

  check_multiple_concrete_definitions
    check_member_unique
    class_
    member_name
    member_kind
    parent_class_elt
    class_elt
    on_error;
  check_compatible_sound_dynamic_attributes
    env
    member_name
    member_kind
    parent_class_elt
    class_elt
    on_error;

  let (lazy fty_child) = class_elt.ce_type in
  let (lazy fty_parent) = parent_class_elt.ce_type in
  match (deref fty_parent, deref fty_child) with
  | ((_, Tany _), (_, Tany _)) -> env
  | ((_, Tany _), _) ->
    Errors.add_typing_error
      Typing_error.(
        apply_reasons ~on_error
        @@ Secondary.Decl_override_missing_hint parent_pos);

    env
  | (_, (_, Tany _)) ->
    Errors.add_typing_error
      Typing_error.(
        apply_reasons ~on_error @@ Secondary.Decl_override_missing_hint pos);
    env
  | ((r_parent, Tfun ft_parent), (r_child, Tfun ft_child)) ->
    (match member_kind with
    | MemberKind.Constructor { is_consistent = false } ->
      (* we don't check that constructor signatures follow
       * subtyping rules except with __ConsistentConstruct *)
      env
    | _ ->
      check_ambiguous_inheritance
        (check_subtype_methods
           env
           ~check_return:(not (MemberKind.is_constructor member_kind))
           on_error)
        (Typing_reason.localize r_parent, ft_parent)
        (Typing_reason.localize r_child, ft_child)
        pos
        class_
        class_elt.ce_origin
        on_error)
  | _ ->
    if get_ce_const class_elt then
      Typing_phase.sub_type_decl env fty_child fty_parent on_error
    else
      Typing_ops.unify_decl
        pos
        Typing_reason.URnone
        env
        on_error
        fty_parent
        fty_child

(* Constants and type constants with declared values in declared interfaces can never be
 * overridden by other inherited constants.
 * Constants from traits are taken into account only if the --enable-strict-const-semantics is enabled
 * @precondition: both constants must not be synthesized
 *)
let conflict_with_declared_interface_or_trait
    ?(include_traits = true)
    env
    implements
    parent_class
    class_
    parent_origin
    origin
    const_name =
  let strict_const_semantics =
    TCO.enable_strict_const_semantics (Env.get_tcopt env)
  in
  let is_inherited_and_conflicts_with_parent =
    String.( <> ) origin (Cls.name class_) && String.( <> ) origin parent_origin
  in
  let child_const_from_used_trait =
    if strict_const_semantics && include_traits then
      match Env.get_class env origin with
      | Some cls -> Cls.kind cls |> Ast_defs.is_c_trait
      | None -> false
    else
      false
  in

  (* True if a declared interface on class_ has a concrete constant with
     the same name and origin as child constant *)
  let child_const_from_declared_interface =
    match Env.get_class env origin with
    | Some cls ->
      Cls.kind cls |> Ast_defs.is_c_interface
      &&
      if strict_const_semantics && include_traits then
        true
      else
        List.fold implements ~init:false ~f:(fun acc (_, iface) ->
            acc
            ||
            match Cls.get_const iface const_name with
            | None -> false
            | Some const -> String.( = ) const.cc_origin origin)
    | None -> false
  in

  match Cls.kind parent_class with
  | Ast_defs.Cinterface -> is_inherited_and_conflicts_with_parent
  | Ast_defs.Cclass _ ->
    is_inherited_and_conflicts_with_parent
    && (child_const_from_declared_interface || child_const_from_used_trait)
  | Ast_defs.Ctrait ->
    is_inherited_and_conflicts_with_parent
    && (child_const_from_declared_interface || child_const_from_used_trait)
    &&
    (* constant must be declared on a trait (or interface if include_traits == true) to conflict *)
    (match Env.get_class env parent_origin with
    | Some cls ->
      if strict_const_semantics && include_traits then
        Cls.kind cls |> fun k ->
        Ast_defs.is_c_trait k || Ast_defs.is_c_interface k
      else
        Cls.kind cls |> Ast_defs.is_c_trait
    | None -> false)
  | Ast_defs.Cenum_class _
  | Ast_defs.Cenum ->
    false

let check_const_override
    env
    implements
    const_name
    parent_class
    class_
    psubst
    parent_class_const
    class_const
    on_error =
  if String.equal parent_class_const.cc_origin class_const.cc_origin then
    env
  else
    let parent_class_const = Inst.instantiate_cc psubst parent_class_const in
    let parent_kind = Cls.kind parent_class in
    let class_kind = Cls.kind class_ in
    (* Shared preconditions for const_interface_member_not_unique and
       is_bad_interface_const_override *)
    let is_concrete = function
      | CCConcrete -> true
      | CCAbstract _ -> false
    in
    let both_are_non_synthetic_and_concrete =
      (* Synthetic  *)
      (not class_const.cc_synthesized)
      (* The parent we are checking is synthetic, no point in checking *)
      && (not parent_class_const.cc_synthesized)
      (* Only check if parent and child have concrete definitions *)
      && is_concrete class_const.cc_abstract
      && is_concrete parent_class_const.cc_abstract
    in
    let const_interface_or_trait_member_not_unique =
      (* Similar to detect_multiple_concrete_defs, we check if there are multiple
         concrete implementations of class constants with no override.
      *)
      conflict_with_declared_interface_or_trait
        env
        implements
        parent_class
        class_
        parent_class_const.cc_origin
        class_const.cc_origin
        const_name
      && both_are_non_synthetic_and_concrete
    in
    let is_bad_interface_const_override =
      (* HHVM does not support one specific case of overriding constants:
         If the original constant was defined as non-abstract in an interface or trait,
         it cannot be overridden when implementing or extending that interface or using that trait. *)
      if Ast_defs.is_c_interface parent_kind then
        both_are_non_synthetic_and_concrete
        (* Check that the constant is indeed defined in class_ *)
        && String.( = ) class_const.cc_origin (Cls.name class_)
      else
        false
    in
    let is_abstract_concrete_override =
      match (parent_class_const.cc_abstract, class_const.cc_abstract) with
      | (CCConcrete, CCAbstract _) -> true
      | _ -> false
    in

    let remove_hh_member_of dty =
      match get_node dty with
      | Tapply (_hh_member_of, [_enum; dty]) -> dty
      | _ -> dty
    in
    let class_const_type =
      if Ast_defs.is_c_enum_class class_kind then
        remove_hh_member_of class_const.cc_type
      else
        class_const.cc_type
    in
    let parent_class_const_type =
      if Ast_defs.is_c_enum_class parent_kind then
        remove_hh_member_of parent_class_const.cc_type
      else
        parent_class_const.cc_type
    in

    let err_opt =
      if const_interface_or_trait_member_not_unique then
        let snd_err =
          Typing_error.Secondary.Interface_or_trait_const_multiple_defs
            {
              pos = class_const.cc_pos;
              name = const_name;
              origin = class_const.cc_origin;
              parent_pos = parent_class_const.cc_pos;
              parent_origin = parent_class_const.cc_origin;
            }
        in
        Some (Typing_error.apply_reasons ~on_error snd_err)
      else if is_bad_interface_const_override then
        let snd_err =
          Typing_error.Secondary.Concrete_const_interface_override
            {
              pos = class_const.cc_pos;
              name = const_name;
              parent_pos = parent_class_const.cc_pos;
              parent_origin = parent_class_const.cc_origin;
            }
        in
        Some (Typing_error.apply_reasons ~on_error snd_err)
      else if is_abstract_concrete_override then
        let snd_err =
          Typing_error.Secondary.Abstract_concrete_override
            {
              pos = class_const.cc_pos;
              parent_pos = parent_class_const.cc_pos;
              kind = `constant;
            }
        in
        Some
          (Typing_error.assert_in_current_decl
             ~ctx:(Env.get_current_decl_and_file env)
             snd_err)
      else
        None
    in
    Option.iter err_opt ~f:Errors.add_typing_error;

    Phase.sub_type_decl
      env
      class_const_type
      parent_class_const_type
      (Typing_error.Reasons_callback.class_constant_type_mismatch on_error)

let check_inherited_member_is_dynamically_callable
    env
    inheriting_class
    parent_class
    (member_kind, member_name, parent_class_elt) =
  let (inheriting_class_pos, inheriting_class) = inheriting_class in
  if
    TypecheckerOptions.enable_sound_dynamic
      (Provider_context.get_tcopt (Env.get_ctx env))
    && Cls.get_support_dynamic_type inheriting_class
    && not (Cls.get_support_dynamic_type parent_class)
    (* TODO: ideally refactor so the last test is not systematically performed on all methods *)
  then
    match Cls.kind parent_class with
    | Ast_defs.Cclass _
    | Ast_defs.Ctrait ->
      begin
        match member_kind with
        | MemberKind.Method ->
          if not (Typing_defs.get_ce_support_dynamic_type parent_class_elt) then
            (* since the attribute is missing run the inter check *)
            let (lazy (ty : decl_ty)) = parent_class_elt.ce_type in
            (match get_node ty with
            | Tfun fun_ty ->
              if
                not
                  (Typing_dynamic.sound_dynamic_interface_check_from_fun_ty
                     env
                     fun_ty)
              then
                Errors.method_is_not_dynamically_callable
                  inheriting_class_pos
                  member_name
                  (Cls.name inheriting_class)
                  false
                  (Some
                     ( Lazy.force parent_class_elt.ce_pos,
                       parent_class_elt.ce_origin ))
                  None
            | _ -> ())
        | MemberKind.Static_method
        | MemberKind.Static_property
        | MemberKind.Property
        | MemberKind.Constructor _ ->
          ()
      end
    | Ast_defs.Cinterface
    | Ast_defs.Cenum_class _
    | Ast_defs.Cenum ->
      ()

let check_class_against_parent_class_elt
    on_error
    (class_pos, class_)
    member_kind
    member_name
    (parent_class_elt, (parent_name_pos, parent_class))
    env =
  match get_member member_kind class_ member_name with
  | Some class_elt ->
    if String.equal parent_class_elt.ce_origin class_elt.ce_origin then (
      (* Case where the child's element comes from the parent being checked. *)
      (* if the child class implements dynamic, all inherited methods should be dynamically callable *)
      check_inherited_member_is_dynamically_callable
        env
        (class_pos, class_)
        parent_class
        (member_kind, member_name, parent_class_elt);
      env
    ) else
      (* We can skip this check if the class elements have the same origin, as we are
         essentially comparing a method against itself *)
      check_override
        ~check_member_unique:
          (detect_multiple_concrete_defs
             class_elt
             class_
             parent_class_elt
             parent_class)
        env
        member_name
        member_kind
        class_
        parent_class
        parent_class_elt
        class_elt
        (on_error (parent_name_pos, Cls.name parent_class))
  | None ->
    (* The only case when a member belongs to a parent but not the child is if the parent is an
       interface and the child is a concrete class. Otherwise, the member would have been inherited.
       In this case, this is an error because the concrete class fails to implement the parent interface. *)
    member_not_implemented_error
      (Cls.name class_)
      (Cls.name parent_class)
      member_name
      parent_class_elt
      class_pos
      parent_name_pos;
    env

let check_members_from_all_parents
    env
    (class_pos, class_)
    (on_error : Pos_or_decl.t * string -> Typing_error.Reasons_callback.t)
    (parent_members : ClassEltWParentSet.t SMap.t MemberKindMap.t) =
  let check member_kind member_map env =
    let check member_name class_elts_w_parents env =
      let check =
        check_class_against_parent_class_elt
          on_error
          (class_pos, class_)
          member_kind
          member_name
      in
      ClassEltWParentSet.fold check class_elts_w_parents env
    in
    SMap.fold check member_map env
  in
  MemberKindMap.fold check parent_members env

let make_all_members ~parent_class =
  let wrap_constructor (ctor, kind) =
    ( MemberKind.Constructor { is_consistent = constructor_is_consistent kind },
      ctor
      |> Option.map ~f:(fun x -> (Naming_special_names.Members.__construct, x))
      |> Option.to_list )
  in
  [
    (MemberKind.Property, Cls.props parent_class);
    (MemberKind.Static_property, Cls.sprops parent_class);
    (MemberKind.Method, Cls.methods parent_class);
    (MemberKind.Static_method, Cls.smethods parent_class);
    Cls.construct parent_class |> wrap_constructor;
  ]

(* The phantom class element that represents the default constructor:
 * public function __construct()[] {}
 *
 * It isn't added to the tc_construct only because that's used to
 * determine whether a child class needs to call parent::__construct *)
let default_constructor_ce class_ =
  let (pos, name) = (Cls.pos class_, Cls.name class_) in
  let r = Reason.Rwitness_from_decl pos in
  (* reason doesn't get used in, e.g. arity checks *)
  let ft =
    {
      ft_tparams = [];
      ft_where_constraints = [];
      ft_params = [];
      ft_implicit_params = { capability = CapTy (MakeType.mixed r) };
      ft_ret = { et_type = MakeType.void r; et_enforced = Unenforced };
      ft_flags = 0;
      ft_ifc_decl = default_ifc_fun_decl;
    }
  in
  {
    ce_visibility = Vpublic;
    ce_type = lazy (mk (r, Tfun ft));
    ce_origin = name;
    ce_deprecated = None;
    ce_pos = lazy pos;
    ce_flags =
      make_ce_flags
        ~xhp_attr:None
        ~abstract:false
        ~final:false
        ~const:false
        ~lateinit:false
        ~superfluous_override:false
        ~lsb:false
        ~synthesized:true
        ~dynamicallycallable:false
        ~readonly_prop:false
        ~support_dynamic_type:false
        ~needs_init:false;
  }

(* When an interface defines a constructor, we check that they are compatible *)
let check_constructors env parent_class class_ psubst on_error =
  let parent_is_interface = Ast_defs.is_c_interface (Cls.kind parent_class) in
  let parent_is_consistent =
    constructor_is_consistent (snd (Cls.construct parent_class))
  in
  if parent_is_interface || parent_is_consistent then
    match (fst (Cls.construct parent_class), fst (Cls.construct class_)) with
    | (Some parent_cstr, _) when get_ce_synthesized parent_cstr -> env
    | (Some parent_cstr, None) ->
      let (lazy pos) = parent_cstr.ce_pos in
      Errors.add_typing_error
        Typing_error.(
          apply_reasons ~on_error @@ Secondary.Missing_constructor pos);
      env
    | (_, Some cstr) when get_ce_superfluous_override cstr ->
      (* <<__UNSAFE_Construct>> *)
      env
    | (opt_parent_cstr, Some cstr)
      when Option.is_some opt_parent_cstr || parent_is_consistent ->
      let parent_cstr =
        match opt_parent_cstr with
        | Some parent_cstr -> parent_cstr
        | None -> default_constructor_ce parent_class
      in
      if String.( <> ) parent_cstr.ce_origin cstr.ce_origin then
        let parent_cstr = Inst.instantiate_ce psubst parent_cstr in
        check_override
          env
          ~check_member_unique:false
          Naming_special_names.Members.__construct
          (MemberKind.Constructor { is_consistent = true })
          class_
          parent_class
          parent_cstr
          cstr
          on_error
      else
        env
    | (_, _) -> env
  else (
    begin
      match (fst (Cls.construct parent_class), fst (Cls.construct class_)) with
      | (Some parent_cstr, _) when get_ce_synthesized parent_cstr -> ()
      | (Some parent_cstr, Some child_cstr) ->
        check_override_final_method parent_cstr child_cstr on_error;
        check_class_elt_visibility parent_cstr child_cstr on_error
      | (_, _) -> ()
    end;
    env
  )

(** Checks if a child is compatible with the type constant of its parent.
    This requires the child's constraint and assigned type to be a subtype of
    the parent's type constant. *)
let tconst_subsumption
    env
    class_name
    parent_typeconst
    parent_tconst_enforceable
    child_typeconst
    on_error =
  let (pos, name) = child_typeconst.ttc_name in
  let parent_pos = fst parent_typeconst.ttc_name in
  match (parent_typeconst.ttc_kind, child_typeconst.ttc_kind) with
  | ( TCAbstract { atc_default = Some _; _ },
      TCAbstract { atc_default = None; _ } ) ->
    Errors.add_typing_error
      Typing_error.(
        assert_in_current_decl ~ctx:(Env.get_current_decl_and_file env)
        @@ Secondary.Override_no_default_typeconst { pos; parent_pos });
    env
  | (TCConcrete _, TCAbstract _) ->
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See typecheck/tconst/subsume_tconst5.php test case for example. *)
    Errors.add_typing_error
      Typing_error.(
        assert_in_current_decl ~ctx:(Env.get_current_decl_and_file env)
        @@ Secondary.Abstract_concrete_override
             { pos; parent_pos; kind = `typeconst });
    env
  | _ ->
    let inherited = not (String.equal child_typeconst.ttc_origin class_name) in
    (* If the class element is inherited from a parent class, we must
     * wrap any error with
     *   "Class [class_name] does not correctly implement all required members"
     * and the primary position should be on [class_name]
     *)
    let on_error =
      if inherited then
        on_error
      else
        Env.unify_error_assert_primary_pos_in_current_decl env
    in

    (* Check that the child's constraint is compatible with the parent. If the
     * parent has a constraint then the child must also have a constraint if it
     * is abstract.
     *
     * Check that the child's assigned type satisifies parent constraint
     *)
    let default =
      MakeType.generic (Reason.Rtconst_no_cstr child_typeconst.ttc_name) name
    in
    let is_coeffect =
      parent_typeconst.ttc_is_ctx || child_typeconst.ttc_is_ctx
    in
    let check_cstrs reason env sub super =
      Option.value ~default:env
      @@ Option.map2
           sub
           super
           ~f:(Typing_ops.sub_type_decl ~is_coeffect ~on_error pos reason env)
    in
    (* TODO(T88552052) This can be greatly simplified by adopting the { A = S..T } representation
     * from DOT and implementing the Typ-<:-Typ rule, Amin 2016 *)
    let env =
      match parent_typeconst.ttc_kind with
      | TCAbstract
          {
            atc_as_constraint = p_as_opt;
            atc_super_constraint = p_super_opt;
            _;
          } ->
        begin
          match child_typeconst.ttc_kind with
          | TCAbstract
              {
                atc_as_constraint = c_as_opt;
                atc_super_constraint = c_super_opt;
                _;
              } ->
            (* TODO(T88552052) this transformation can be done with mixed and nothing *)
            let c_as_opt = Some (Option.value c_as_opt ~default) in
            let c_super_opt = Some (Option.value c_super_opt ~default) in

            let env =
              check_cstrs Reason.URsubsume_tconst_cstr env c_as_opt p_as_opt
            in
            check_cstrs Reason.URsubsume_tconst_cstr env p_super_opt c_super_opt
          | TCConcrete { tc_type = c_t } ->
            let env =
              check_cstrs Reason.URtypeconst_cstr env (Some c_t) p_as_opt
            in
            check_cstrs Reason.URtypeconst_cstr env p_super_opt (Some c_t)
        end
      | TCConcrete _ ->
        begin
          match child_typeconst.ttc_kind with
          | TCConcrete _ ->
            if
              TypecheckerOptions.typeconst_concrete_concrete_error
                (Env.get_tcopt env)
              && not inherited
            then
              Errors.add_typing_error
                Typing_error.(
                  assert_in_current_decl
                    ~ctx:(Env.get_current_decl_and_file env)
                  @@ Secondary.Typeconst_concrete_concrete_override
                       { pos; parent_pos })
          | _ -> ()
        end;
        env
    in

    (* Don't recheck inherited type constants: errors will
     * have been emitted already for the parent *)
    (if inherited then
      ()
    else
      match (child_typeconst.ttc_kind, parent_tconst_enforceable) with
      | (TCAbstract { atc_default = Some ty; _ }, (tp_pos, true))
      | (TCConcrete { tc_type = ty }, (tp_pos, true)) ->
        let emit_error pos ty_info =
          Errors.add_typing_error
            Typing_error.(
              primary
              @@ Primary.Invalid_enforceable_type
                   { pos; ty_info; kind = `constant; tp_pos; tp_name = name })
        in
        Typing_enforceable_hint.validate_type
          env
          (fst child_typeconst.ttc_name |> Pos_or_decl.unsafe_to_raw_pos)
          ty
          emit_error
      | _ ->
        ();

        (match parent_typeconst.ttc_reifiable with
        | None -> ()
        | Some pos ->
          Typing_const_reifiable.check_reifiable env child_typeconst pos));

    (* If the parent cannot be overridden, we unify the types otherwise we ensure
     * the child's assigned type is compatible with the parent's
     *
     * TODO(T88552052) restrict concrete typeconst overriding
     *)
    let parent_is_final =
      match parent_typeconst.ttc_kind with
      | TCConcrete _ -> true
      | TCAbstract _ -> false
    in
    let check env x y =
      if parent_is_final then
        Typing_ops.unify_decl
          pos
          Reason.URsubsume_tconst_assign
          env
          on_error
          x
          y
      else
        Typing_ops.sub_type_decl
          ~on_error
          pos
          Reason.URsubsume_tconst_assign
          env
          y
          x
    in
    (* TODO(T88552052) this fetching of types is a temporary hack; this whole check will be eliminated *)
    let opt_type__LEGACY t =
      match t.ttc_kind with
      | TCConcrete { tc_type = t } -> Some t
      | TCAbstract _ -> None
    in
    Option.value ~default:env
    @@ Option.map2
         (opt_type__LEGACY parent_typeconst)
         (opt_type__LEGACY child_typeconst)
         ~f:(check env)

let check_typeconst_override
    env implements class_ parent_tconst tconst parent_class on_error =
  if String.equal parent_tconst.ttc_origin tconst.ttc_origin then
    env
  else
    let tconst_check parent_tconst tconst () =
      let parent_tconst_enforceable =
        (* We know that this typeconst exists in the parent (else we would not
           have successfully looked up [parent_tconst]), so we know that
           [get_typeconst_enforceability] will return Some. *)
        Option.value_exn
          (Cls.get_typeconst_enforceability parent_class (snd tconst.ttc_name))
      in
      tconst_subsumption
        env
        (Cls.name class_)
        parent_tconst
        parent_tconst_enforceable
        tconst
        on_error
    in
    let env =
      check_ambiguous_inheritance
        tconst_check
        parent_tconst
        tconst
        (fst tconst.ttc_name)
        class_
        tconst.ttc_origin
        on_error
    in
    let (pos, name) = tconst.ttc_name in
    let parent_pos = fst parent_tconst.ttc_name in
    (* Temporarily skip checks on context constants
     *
     * TODO(T89366955) elimninate this check *)
    let is_context_constant =
      match (parent_tconst.ttc_kind, tconst.ttc_kind) with
      | ( TCAbstract { atc_default = Some hint1; _ },
          TCAbstract { atc_default = Some hint2; _ } ) ->
        (match (deref hint1, deref hint2) with
        | ((_, Tintersection _), _)
        | (_, (_, Tintersection _)) ->
          true
        | _ -> false)
      | (TCAbstract { atc_default = Some hint; _ }, _)
      | (_, TCAbstract { atc_default = Some hint; _ }) ->
        (match deref hint with
        | (_, Tintersection _) -> true
        | _ -> false)
      | _ -> false
    in
    (match (parent_tconst.ttc_kind, tconst.ttc_kind) with
    | (TCConcrete _, TCConcrete _)
    | (TCAbstract { atc_default = Some _; _ }, TCConcrete _)
    | ( TCAbstract { atc_default = Some _; _ },
        TCAbstract { atc_default = Some _; _ } ) ->
      if
        (not is_context_constant)
        && (not tconst.ttc_synthesized)
        && (not parent_tconst.ttc_synthesized)
        && conflict_with_declared_interface_or_trait
             ~include_traits:false
             env
             implements
             parent_class
             class_
             parent_tconst.ttc_origin
             tconst.ttc_origin
             name
      then
        let child_is_abstract =
          match tconst.ttc_kind with
          | TCConcrete _ -> false
          | TCAbstract _ -> true
        in
        let err =
          Typing_error.Secondary.Interface_typeconst_multiple_defs
            {
              pos;
              name;
              is_abstract = child_is_abstract;
              origin = tconst.ttc_origin;
              parent_pos;
              parent_origin = parent_tconst.ttc_origin;
            }
        in
        Errors.add_typing_error @@ Typing_error.(apply_reasons ~on_error err)
    | _ -> ());
    env

(** For type constants we need to check that a child respects the
    constraints specified by its parent, and does not conflict
    with other inherited type constants *)
let check_typeconsts
    env
    implements
    (parent_class : pos_id * decl_ty list * Cls.t)
    class_
    on_error =
  let ((parent_pos, _), _, parent_class) = parent_class in
  let (pos, class_) = class_ in
  let ptypeconsts = Cls.typeconsts parent_class in
  List.fold ptypeconsts ~init:env ~f:(fun env (tconst_name, parent_tconst) ->
      match Cls.get_typeconst class_ tconst_name with
      | Some tconst ->
        check_typeconst_override
          env
          implements
          class_
          parent_tconst
          tconst
          parent_class
          on_error
      | None ->
        (* The only case when a member belongs to a parent but not the child is if the parent is an
           interface and the child is a concrete class. Otherwise, the member would have been inherited. *)
        let err =
          Typing_error.(
            primary
            @@ Primary.Member_not_implemented
                 {
                   member_name = tconst_name;
                   parent_pos;
                   pos;
                   decl_pos = fst parent_tconst.ttc_name;
                   quickfixes = [];
                 })
        in
        Errors.add_typing_error err;
        env)

let check_consts env implements parent_class (name_pos, class_) psubst on_error
    =
  let pconsts = Cls.consts parent_class in
  List.fold pconsts ~init:env ~f:(fun env (const_name, parent_const) ->
      if String.( <> ) const_name SN.Members.mClass then (
        match Cls.get_const class_ const_name with
        | Some const ->
          (* skip checks for typeconst derived class constants *)
          (match Cls.get_typeconst class_ const_name with
          | None ->
            check_const_override
              env
              implements
              const_name
              parent_class
              class_
              psubst
              parent_const
              const
              on_error
          | Some _ -> env)
        | None ->
          let err =
            Typing_error.(
              primary
              @@ Primary.Member_not_implemented
                   {
                     member_name = const_name;
                     parent_pos = parent_const.cc_pos;
                     pos = name_pos;
                     decl_pos = Cls.pos parent_class;
                     quickfixes = [];
                   })
          in
          Errors.add_typing_error err;
          env
      ) else
        env)

(* Use the [on_error] callback if we need to wrap the basic error with a
 *   "Class ... does not correctly implement all required members"
 * message pointing at the class being checked.
 *)
let check_class_extends_parent
    env
    implements
    (parent_class : pos_id * decl_ty list * Cls.t)
    (name_pos, class_)
    on_error =
  let env =
    check_typeconsts env implements parent_class (name_pos, class_) on_error
  in
  let (_, parent_tparaml, parent_class) = parent_class in
  let psubst = Inst.make_subst (Cls.tparams parent_class) parent_tparaml in
  let env =
    check_consts env implements parent_class (name_pos, class_) psubst on_error
  in
  let env = check_constructors env parent_class class_ psubst on_error in
  env

(** Eliminate all synthesized members (those from requirements) plus
all private members unless they're from traits. *)
let filter_privates_and_synthethized
    ~(is_trait : bool) (members : ('a * class_elt) list) : ('a * class_elt) list
    =
  let eliminate class_elt =
    get_ce_synthesized class_elt
    || ((not is_trait) && Typing_defs.class_elt_is_private_not_lsb class_elt)
  in
  let keep class_elt = not (eliminate class_elt) in
  List.filter members ~f:(fun (_name, class_elt) -> keep class_elt)

let make_parent_member_map
    ((parent_name_pos, _parent_name), parent_tparaml, parent_class) :
    ClassEltWParentSet.t SMap.t MemberKindMap.t =
  let psubst = Inst.make_subst (Cls.tparams parent_class) parent_tparaml in
  make_all_members ~parent_class
  |> MemberKindMap.of_list
  |> MemberKindMap.map (fun members ->
         members
         |> filter_privates_and_synthethized
              ~is_trait:(Ast_defs.is_c_trait (Cls.kind parent_class))
         |> SMap.of_list
         |> SMap.map (fun member ->
                let member : class_elt = Inst.instantiate_ce psubst member in
                ClassEltWParentSet.singleton
                  (member, (parent_name_pos, parent_class))))

let merge_member_maps acc_map map =
  MemberKindMap.fold
    (fun mem_kind members acc_map ->
      let new_entry =
        match MemberKindMap.find_opt mem_kind acc_map with
        | None -> members
        | Some prev_members ->
          let new_members =
            SMap.merge
              (fun _member_name -> Option.merge ~f:ClassEltWParentSet.union)
              prev_members
              members
          in
          new_members
      in
      MemberKindMap.add mem_kind new_entry acc_map)
    map
    acc_map

let fold_parent_members parents : ClassEltWParentSet.t SMap.t MemberKindMap.t =
  parents
  |> List.map ~f:make_parent_member_map
  |> List.fold ~init:MemberKindMap.empty ~f:merge_member_maps

let check_class_extends_parents
    env
    (class_name_pos, class_)
    (parents : (pos_id * decl_ty list * Cls.t) list)
    (on_error : Pos_or_decl.t * string -> Typing_error.Reasons_callback.t) =
  let members : ClassEltWParentSet.t SMap.t MemberKindMap.t =
    fold_parent_members parents
  in
  check_members_from_all_parents env (class_name_pos, class_) on_error members

(*****************************************************************************)
(* The externally visible function *)
(*****************************************************************************)

(* [parents] also contains traits.
  Here's a simple example showing why we need to check overriding traits:

    trait T {
      public function foo(): void {}
      public function bar(): void {
        $this->foo();
      }
    }

    class A {
      use T;
      public function foo(int $x): void {}
    }

    Overriding foo this way is unsound due to bar using foo,
    so A::foo needs to be a subtype of T::foo.
  *)
let check_implements_extends_uses env ~implements ~parents (name_pos, class_) =
  let implements =
    let decl_ty_to_cls x =
      let (_, (pos, name), _) = TUtils.unwrap_class_type x in
      Env.get_class env name >>| fun class_ -> (pos, class_)
    in
    List.filter_map implements ~f:decl_ty_to_cls
  in
  let on_error (parent_name_pos, parent_name) : Typing_error.Reasons_callback.t
      =
    (* sadly, enum error reporting requires this to keep the error in the file
       with the enum *)
    if String.equal parent_name SN.Classes.cHH_BuiltinEnum then
      Typing_error.Reasons_callback.bad_enum_decl name_pos
    else
      Typing_error.Reasons_callback.bad_decl_override
        name_pos
        ~name:(Cls.name class_)
        ~parent_pos:parent_name_pos
        ~parent_name
  in
  let parents =
    let destructure_type ty =
      let (_, name, tparaml) = TUtils.unwrap_class_type ty in
      Env.get_class env (snd name) >>| fun class_ -> (name, tparaml, class_)
    in
    List.filter_map parents ~f:destructure_type
  in
  let env =
    List.fold ~init:env parents ~f:(fun env ((parent_name, _, _) as parent) ->
        check_class_extends_parent
          env
          implements
          parent
          (name_pos, class_)
          (on_error parent_name))
  in
  let env =
    check_class_extends_parents env (name_pos, class_) parents on_error
  in
  env
