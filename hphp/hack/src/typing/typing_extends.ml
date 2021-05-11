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
open Typing_defs
module Env = Typing_env
module Dep = Typing_deps.Dep
module TUtils = Typing_utils
module Inst = Decl_instantiate
module Phase = Typing_phase
module SN = Naming_special_names
module Cls = Decl_provider.Class
module MakeType = Typing_make_type

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let is_private = function
  | { ce_visibility = Vprivate _; _ } -> true
  | _ -> false

let is_lsb ce = get_ce_lsb ce

(*****************************************************************************)
(* Given a map of members, check that the overriding is correct.
 * Please note that 'members' has a very general meaning here.
 * It can be class variables, methods, static methods etc ... The same logic
 * is applied to verify that the overriding is correct.
 *)
(*****************************************************************************)

let use_parent_for_known = false

(* Rules for visibility *)
let check_visibility parent_vis c_vis parent_pos pos on_error =
  match (parent_vis, c_vis) with
  | (Vprivate _, _) ->
    (* The only time this case should come into play is when the
     * parent_class_elt comes from a trait *)
    ()
  | (Vpublic, Vpublic)
  | (Vprotected _, Vprotected _)
  | (Vprotected _, Vpublic) ->
    ()
  | _ ->
    let parent_vis = TUtils.string_of_visibility parent_vis in
    let vis = TUtils.string_of_visibility c_vis in
    Errors.visibility_extends vis pos parent_pos parent_vis on_error

let check_class_elt_visibility parent_class_elt class_elt on_error =
  let parent_vis = parent_class_elt.ce_visibility in
  let c_vis = class_elt.ce_visibility in
  let (lazy parent_pos) = parent_class_elt.ce_pos in
  let (lazy pos) = class_elt.ce_pos in
  check_visibility parent_vis c_vis parent_pos pos on_error

(* Check that all the required members are implemented *)
let check_members_implemented
    check_private parent_reason reason (_, parent_members, get_member) =
  List.iter parent_members (fun (member_name, class_elt) ->
      match class_elt.ce_visibility with
      | Vprivate _ when not check_private -> ()
      | Vprivate _ ->
        (* This case cannot be removed as long as we're forced to
         * check against every extended parent by the fact that // decl
         * parents aren't fully checked against grandparents; when
         * (class) extends (class // decl) use (trait), the grandchild
         * won't have access to private members of the grandparent
         * trait *)
        ()
      | _ when Option.is_none (get_member member_name) ->
        let (lazy defn_pos) = class_elt.ce_pos in
        Errors.member_not_implemented member_name parent_reason reason defn_pos
      | _ -> ())

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
    ~when_:(fun () ->
      String.( <> ) (Cls.name class_) origin
      && Errors.has_no_errors (f child parent))
    ~do_:(fun error ->
      Errors.ambiguous_inheritance pos (Cls.name class_) origin error on_error)

let should_check_params parent_class class_ =
  if use_parent_for_known then
    Cls.members_fully_known parent_class
  else
    Cls.members_fully_known class_

let is_method member_source =
  match member_source with
  | `FromMethod
  | `FromSMethod ->
    true
  | _ -> false

let check_final_method parent_class_elt class_elt on_error =
  let is_override =
    get_ce_final parent_class_elt
    && String.( <> ) parent_class_elt.ce_origin class_elt.ce_origin
  in
  if is_override && not (get_ce_synthesized class_elt) then
    (* we have a final method being overridden by a user-declared method *)
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    let (lazy pos) = class_elt.ce_pos in
    Errors.override_final ~parent:parent_pos ~child:pos ~on_error

let check_dynamically_callable member_name parent_class_elt class_elt on_error =
  if
    get_ce_dynamicallycallable parent_class_elt
    && not (get_ce_dynamicallycallable class_elt)
  then
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    let (lazy pos) = class_elt.ce_pos in
    let errorl =
      [
        (parent_pos, "This method is `__DynamicallyCallable`.");
        (pos, "This method is **not**.");
      ]
    in
    Errors.bad_method_override pos member_name errorl on_error

let check_lsb_overrides
    member_source member_name parent_class_elt class_elt on_error =
  let is_sprop =
    match member_source with
    | `FromSProp -> true
    | _ -> false
  in
  let parent_is_lsb = get_ce_lsb parent_class_elt in
  if is_sprop && parent_is_lsb then
    (* __LSB attribute is being overridden *)
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    let (lazy pos) = class_elt.ce_pos in
    Errors.override_lsb member_name parent_pos pos on_error

let check_lateinit parent_class_elt class_elt on_error =
  let lateinit_diff =
    Bool.( <> ) (get_ce_lateinit parent_class_elt) (get_ce_lateinit class_elt)
  in
  if lateinit_diff then
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    let (lazy child_pos) = class_elt.ce_pos in
    Errors.bad_lateinit_override
      (get_ce_lateinit parent_class_elt)
      parent_pos
      child_pos
      on_error

let check_xhp_attr_required env parent_class_elt class_elt on_error =
  if not (TypecheckerOptions.check_xhp_attribute (Env.get_tcopt env)) then
    ()
  else
    let is_less_strict = function
      | (Some Required, _)
      | (Some Lateinit, Some Lateinit)
      | (Some Lateinit, None)
      | (None, None) ->
        false
      | (_, _) -> true
    in
    let parent_attr = get_ce_xhp_attr parent_class_elt in
    let attr = get_ce_xhp_attr class_elt in
    match (parent_attr, attr) with
    | (Some { xa_tag = parent_tag; _ }, Some { xa_tag = tag; _ })
      when is_less_strict (tag, parent_tag) ->
      let (lazy parent_pos) = parent_class_elt.ce_pos in
      let (lazy child_pos) = class_elt.ce_pos in
      let lateinit = Markdown_lite.md_codify "@lateinit" in
      let required = Markdown_lite.md_codify "@required" in
      let show = function
        | None -> Printf.sprintf "not %s or %s" required lateinit
        | Some Required -> required
        | Some Lateinit -> lateinit
      in
      Errors.bad_xhp_attr_required_override
        (show parent_tag)
        (show tag)
        parent_pos
        child_pos
        on_error
    | (_, _) -> ()

(* Check that overriding is correct *)
let check_override
    env
    ~check_member_unique
    member_name
    mem_source
    ?(ignore_fun_return = false)
    parent_class
    class_
    parent_class_elt
    class_elt
    on_error =
  (* If the class element is defined in the class that we're checking, then
   * don't wrap with the extra
   * "Class ... does not correctly implement all required members" message *)
  let on_error =
    if String.equal class_elt.ce_origin (Cls.name class_) then
      Env.unify_error_assert_primary_pos_in_current_decl env
    else
      on_error
  in
  let check_compatible_sound_dynamic_attributes
      member_name parent_class (class_ : Cls.t) parent_class_elt class_elt =
    if
      TypecheckerOptions.enable_sound_dynamic
        (Provider_context.get_tcopt (Env.get_ctx env))
      && ( Cls.get_support_dynamic_type parent_class
         || get_ce_support_dynamic_type parent_class_elt )
      && not
           ( Cls.get_support_dynamic_type class_
           || get_ce_support_dynamic_type class_elt )
    then
      let (lazy pos) = class_elt.ce_pos in
      let (lazy parent_pos) = parent_class_elt.ce_pos in
      Errors.override_method_support_dynamic_type
        pos
        parent_pos
        parent_class_elt.ce_origin
        member_name
        on_error
  in

  if is_method mem_source then begin
    (* We first verify that we aren't overriding a final method *)
    (* we only check for final overrides on methods, not properties *)
    (* we don't check constructors, as they are already checked
     * in the decl phase *)
    check_final_method parent_class_elt class_elt on_error;
    check_dynamically_callable member_name parent_class_elt class_elt on_error
  end;

  (* Verify that we are not overriding an __LSB property *)
  check_lsb_overrides mem_source member_name parent_class_elt class_elt on_error;
  check_lateinit parent_class_elt class_elt on_error;
  check_xhp_attr_required env parent_class_elt class_elt on_error;
  let check_params = should_check_params parent_class class_ in
  check_class_elt_visibility parent_class_elt class_elt on_error;
  let (lazy pos) = class_elt.ce_pos in
  let (lazy parent_pos) = parent_class_elt.ce_pos in
  let is_method =
    match mem_source with
    | `FromMethod
    | `FromSMethod
    | `FromConstructor ->
      true
    | `FromProp
    | `FromSProp ->
      false
  in

  if Bool.( <> ) (get_ce_const class_elt) (get_ce_const parent_class_elt) then
    Errors.overriding_prop_const_mismatch
      parent_pos
      (get_ce_const parent_class_elt)
      pos
      (get_ce_const class_elt)
      on_error;
  if (not (get_ce_abstract parent_class_elt)) && get_ce_abstract class_elt then
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See override_abstract_concrete.php test case for example. *)
    Errors.abstract_concrete_override
      pos
      parent_pos
      ( if is_method then
        `method_
      else
        `property )
      ~current_decl_and_file:(Env.get_current_decl_and_file env);
  if check_params then (
    let on_error ?code:_ reasons =
      ( if is_method then
        Errors.bad_method_override
      else
        Errors.bad_prop_override )
        pos
        member_name
        reasons
        on_error
    in
    let (lazy fty_child) = class_elt.ce_type in
    let (lazy fty_parent) = parent_class_elt.ce_type in
    if
      check_member_unique
      && (is_method || get_ce_const class_elt)
      && (not (get_ce_abstract parent_class_elt))
      && not (get_ce_abstract class_elt)
    then
      (* Multiple concrete trait definitions, error *)
      Errors.multiple_concrete_defs
        pos
        parent_pos
        class_elt.ce_origin
        parent_class_elt.ce_origin
        member_name
        (Cls.name class_)
        on_error;
    match (deref fty_parent, deref fty_child) with
    | ((_, Tany _), (_, Tany _)) -> env
    | ((_, Tany _), _) ->
      Errors.decl_override_missing_hint parent_pos on_error;
      env
    | (_, (_, Tany _)) ->
      Errors.decl_override_missing_hint pos on_error;
      env
    | ((r_parent, Tfun ft_parent), (r_child, Tfun ft_child)) ->
      let check (r1, ft1) (r2, ft2) () =
        match mem_source with
        | `FromConstructor ->
          (* we don't check that constructor signatures follow
           * subtyping rules except with __ConsistentConstruct,
           * which is checked elsewhere *)
          env
        | _ ->
          check_compatible_sound_dynamic_attributes
            member_name
            parent_class
            class_
            parent_class_elt
            class_elt;
          Typing_subtype_method.(
            (* Add deps here when we override *)
            subtype_method_decl
              ~check_return:(not ignore_fun_return)
              env
              r2
              ft2
              r1
              ft1
              on_error)
      in
      check_ambiguous_inheritance
        check
        (Typing_reason.localize r_parent, ft_parent)
        (Typing_reason.localize r_child, ft_child)
        pos
        class_
        class_elt.ce_origin
        on_error
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
  ) else
    env

(* Constants and type constants with declared values in declared interfaces can never be
 * overridden by other inherited constants.
 * @precondition: both constants must not be synthesized
 *)
let conflict_with_declared_interface
    env implements parent_class class_ parent_origin origin const_name =
  let is_inherited_and_conflicts_with_parent =
    String.( <> ) origin (Cls.name class_) && String.( <> ) origin parent_origin
  in
  (* True if a declared interface on class_ has a concrete constant with
     the same name and origin as child constant *)
  let child_const_from_declared_interface =
    match Env.get_class env origin with
    | Some cls ->
      Cls.kind cls |> Ast_defs.is_c_interface
      && List.fold implements ~init:false ~f:(fun acc iface ->
             acc
             ||
             match Cls.get_const iface const_name with
             | None -> false
             | Some const -> String.( = ) const.cc_origin origin)
    | None -> false
  in
  match Cls.kind parent_class with
  | Ast_defs.Cinterface -> is_inherited_and_conflicts_with_parent
  | Ast_defs.Cabstract
  | Ast_defs.Cnormal ->
    is_inherited_and_conflicts_with_parent
    && child_const_from_declared_interface
  | Ast_defs.Ctrait ->
    is_inherited_and_conflicts_with_parent
    && child_const_from_declared_interface
    &&
    (* constant must be declared on a trait to conflict *)
    (match Env.get_class env parent_origin with
    | Some cls -> Cls.kind cls |> Ast_defs.is_c_trait
    | None -> false)
  | Ast_defs.Cenum -> false

let check_const_override
    env
    implements
    const_name
    parent_class
    class_
    parent_class_const
    class_const
    on_error =
  let check_params = should_check_params parent_class class_ in
  (* Shared preconditons for const_interface_member_not_unique and
     is_bad_interface_const_override *)
  let both_are_non_synthetic_and_concrete =
    (* Synthetic  *)
    (not class_const.cc_synthesized)
    (* The parent we are checking is synthetic, no point in checking *)
    && (not parent_class_const.cc_synthesized)
    (* Only check if parent and child have concrete definitions *)
    && (not class_const.cc_abstract)
    && not parent_class_const.cc_abstract
  in
  let const_interface_member_not_unique =
    (* Similar to should_check_member_unique, we check if there are multiple
      concrete implementations of class constants with no override.
    *)
    conflict_with_declared_interface
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
     If the original constant was defined as non-abstract in an interface,
     it cannot be overridden when implementing or extending that interface. *)
    match Cls.kind parent_class with
    | Ast_defs.Cinterface ->
      both_are_non_synthetic_and_concrete
      (* Check that the constant is indeed defined in class_ *)
      && String.( = ) class_const.cc_origin (Cls.name class_)
    | Ast_defs.(Cabstract | Cnormal | Ctrait | Cenum) -> false
  in
  let is_abstract_concrete_override =
    (not parent_class_const.cc_abstract) && class_const.cc_abstract
  in

  if check_params then
    if const_interface_member_not_unique then
      Errors.interface_const_multiple_defs
        class_const.cc_pos
        parent_class_const.cc_pos
        class_const.cc_origin
        parent_class_const.cc_origin
        const_name
        on_error
    else if is_bad_interface_const_override then
      Errors.concrete_const_interface_override
        class_const.cc_pos
        parent_class_const.cc_pos
        parent_class_const.cc_origin
        const_name
        on_error
    else if is_abstract_concrete_override then
      Errors.abstract_concrete_override
        class_const.cc_pos
        parent_class_const.cc_pos
        `constant
        ~current_decl_and_file:(Env.get_current_decl_and_file env);
  Phase.sub_type_decl
    env
    class_const.cc_type
    parent_class_const.cc_type
    (Errors.class_constant_type_mismatch on_error)

(* Privates are only visible in the parent, we don't need to check them *)
let filter_privates members =
  List.filter members (fun (_name, class_elt) ->
      (not (is_private class_elt)) || is_lsb class_elt)

let check_members
    check_private
    env
    (parent_class, psubst)
    (class_pos, class_, subst)
    on_error
    (mem_source, parent_members, get_member) =
  let parent_members =
    if check_private then
      parent_members
    else
      filter_privates parent_members
  in
  let should_check_member_unique class_elt parent_class_elt =
    (* We want to check if there are conflicting trait or interface declarations
     * of a class member. This means that if the parent class is a trait or interface,
     * we need to check that the child member is *uniquely inherited*.
     *
     * A member is uniquely inherited if any of the following hold:
     * 1. It is synthetic (from a requirement)
     * 2. It is defined on the child class
     * 3. It is concretely defined in exactly one place
     * 4. It is abstract, and all other declarations are identical *)
    match Cls.kind parent_class with
    | Ast_defs.Cinterface
    | Ast_defs.Ctrait ->
      (* Synthetic  *)
      (not (get_ce_synthesized class_elt))
      (* The parent we are checking is synthetic, no point in checking *)
      && (not (get_ce_synthesized parent_class_elt))
      (* defined on original class *)
      && String.( <> ) class_elt.ce_origin (Cls.name class_)
    | _ -> false
  in
  List.fold
    ~init:env
    parent_members
    ~f:(fun env (member_name, parent_class_elt) ->
      match get_member member_name with
      (* We can skip this check if the class elements have the same origin, as we are
       * essentially comparing a method against itself *)
      | Some class_elt
        when String.( <> ) parent_class_elt.ce_origin class_elt.ce_origin ->
        let parent_class_elt = Inst.instantiate_ce psubst parent_class_elt in
        let class_elt = Inst.instantiate_ce subst class_elt in
        let check_member_unique =
          should_check_member_unique class_elt parent_class_elt
        in
        let dep =
          match mem_source with
          | `FromMethod -> Dep.Method (parent_class_elt.ce_origin, member_name)
          | `FromSMethod -> Dep.SMethod (parent_class_elt.ce_origin, member_name)
          | `FromSProp -> Dep.SProp (parent_class_elt.ce_origin, member_name)
          | `FromProp -> Dep.Prop (parent_class_elt.ce_origin, member_name)
          | `FromConstructor -> Dep.Cstr parent_class_elt.ce_origin
        in
        if not (Pos_or_decl.is_hhi (Cls.pos parent_class)) then
          Typing_deps.add_idep
            (Env.get_deps_mode env)
            (Dep.Type (Cls.name class_))
            dep;
        check_override
          ~check_member_unique
          env
          member_name
          mem_source
          parent_class
          class_
          parent_class_elt
          class_elt
          on_error
      | _ ->
        (* if class implements dynamic, all inherited methods should be dynamically callable *)
        ( if
          TypecheckerOptions.enable_sound_dynamic
            (Provider_context.get_tcopt (Env.get_ctx env))
          && Cls.get_support_dynamic_type class_
          && not (Cls.get_support_dynamic_type parent_class)
          (* TODO: ideally refactor so the last test is not systematically performed on all methods *)
        then
          match Cls.kind parent_class with
          | Ast_defs.Cabstract
          | Ast_defs.Cnormal
          | Ast_defs.Ctrait ->
            begin
              match mem_source with
              | `FromMethod ->
                if
                  not (Typing_defs.get_ce_support_dynamic_type parent_class_elt)
                then
                  (* since the attribute is missing run the inter check *)
                  let (lazy (ty : decl_ty)) = parent_class_elt.ce_type in
                  (match get_node ty with
                  | Tfun fun_ty ->
                    if
                      not
                        (Typing_dynamic
                         .sound_dynamic_interface_check_from_fun_ty
                           env
                           fun_ty)
                    then
                      Errors.method_is_not_dynamically_callable
                        class_pos
                        member_name
                        (Cls.name class_)
                        false
                        (Some
                           ( Lazy.force parent_class_elt.ce_pos,
                             parent_class_elt.ce_origin ))
                        None
                  | _ -> ())
              | `FromSMethod
              | `FromSProp
              | `FromProp
              | `FromConstructor ->
                ()
            end
          | Ast_defs.Cinterface
          | Ast_defs.Cenum ->
            () );
        env)

(*****************************************************************************)
(* Before checking that a class implements an interface, we have to
 * substitute the type parameters with their real type.
 *)
(*****************************************************************************)

(* Instantiation basically applies the substitution *)
let instantiate_consts subst consts =
  List.map consts (fun (id, cc) -> (id, Inst.instantiate_cc subst cc))

let make_all_members ~child_class ~parent_class =
  let wrap_constructor = function
    | None -> []
    | Some x -> [(Naming_special_names.Members.__construct, x)]
  in
  [
    (`FromProp, Cls.props parent_class, Cls.get_prop child_class);
    (`FromSProp, Cls.sprops parent_class, Cls.get_sprop child_class);
    (`FromMethod, Cls.methods parent_class, Cls.get_method child_class);
    (`FromSMethod, Cls.smethods parent_class, Cls.get_smethod child_class);
    ( `FromConstructor,
      fst (Cls.construct parent_class) |> wrap_constructor,
      (fun _ -> fst (Cls.construct child_class)) );
  ]

(* The phantom class element that represents the default constructor:
 * public function __construct() {}
 *
 * It isn't added to the tc_construct only because that's used to
 * determine whether a child class needs to call parent::__construct *)
let default_constructor_ce class_ =
  let (pos, name) = (Cls.pos class_, Cls.name class_) in
  let r = Reason.Rwitness_from_decl pos in
  (* reason doesn't get used in, e.g. arity checks *)
  let ft =
    {
      ft_arity = Fstandard;
      ft_tparams = [];
      ft_where_constraints = [];
      ft_params = [];
      ft_implicit_params = { capability = CapDefaults pos };
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
        ~override:false
        ~lsb:false
        ~synthesized:true
        ~dynamicallycallable:false
        ~readonly_prop:false
        ~support_dynamic_type:false;
  }

(* When an interface defines a constructor, we check that they are compatible *)
let check_constructors env parent_class class_ psubst subst on_error =
  let consistent =
    not (equal_consistent_kind (snd (Cls.construct parent_class)) Inconsistent)
  in
  if
    Ast_defs.(equal_class_kind (Cls.kind parent_class) Cinterface) || consistent
  then
    match (fst (Cls.construct parent_class), fst (Cls.construct class_)) with
    | (Some parent_cstr, _) when get_ce_synthesized parent_cstr -> env
    | (Some parent_cstr, None) ->
      let (lazy pos) = parent_cstr.ce_pos in
      Errors.missing_constructor pos on_error;
      env
    | (_, Some cstr) when get_ce_override cstr ->
      (* <<__UNSAFE_Construct>> *)
      env
    | (opt_parent_cstr, Some cstr)
      when Option.is_some opt_parent_cstr || consistent ->
      let parent_cstr =
        match opt_parent_cstr with
        | Some parent_cstr -> parent_cstr
        | None -> default_constructor_ce parent_class
      in
      if String.( <> ) parent_cstr.ce_origin cstr.ce_origin then begin
        let parent_cstr = Inst.instantiate_ce psubst parent_cstr in
        let cstr = Inst.instantiate_ce subst cstr in
        if not (Pos_or_decl.is_hhi (Cls.pos parent_class)) then
          Typing_deps.add_idep
            (Env.get_deps_mode env)
            (Dep.Type (Cls.name class_))
            (Dep.Cstr parent_cstr.ce_origin);
        check_override
          env
          ~check_member_unique:false
          "__construct"
          `FromMethod
          ~ignore_fun_return:true
          parent_class
          class_
          parent_cstr
          cstr
          on_error
      end else
        env
    | (_, _) -> env
  else (
    begin
      match (fst (Cls.construct parent_class), fst (Cls.construct class_)) with
      | (Some parent_cstr, _) when get_ce_synthesized parent_cstr -> ()
      | (Some parent_cstr, Some child_cstr) ->
        check_final_method parent_cstr child_cstr on_error;
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
    Errors.override_no_default_typeconst
      pos
      parent_pos
      ~current_decl_and_file:(Env.get_current_decl_and_file env);
    env
  | ((TCConcrete _ | TCPartiallyAbstract _), TCAbstract _) ->
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See typecheck/tconst/subsume_tconst5.php test case for example. *)
    Errors.abstract_concrete_override
      pos
      parent_pos
      `typeconst
      ~current_decl_and_file:(Env.get_current_decl_and_file env);
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
          | TCPartiallyAbstract { patc_constraint = c_as; patc_type = c_t } ->
            let env =
              check_cstrs Reason.URsubsume_tconst_cstr env (Some c_as) p_as_opt
            in
            let env =
              check_cstrs Reason.URtypeconst_cstr env (Some c_t) p_as_opt
            in
            check_cstrs Reason.URtypeconst_cstr env p_super_opt (Some c_t)
          | TCConcrete { tc_type = c_t } ->
            let env =
              check_cstrs Reason.URtypeconst_cstr env (Some c_t) p_as_opt
            in
            check_cstrs Reason.URtypeconst_cstr env p_super_opt (Some c_t)
        end
      | TCPartiallyAbstract { patc_constraint = p_as; _ } ->
        (* TODO(T88552052) Can do abstract_concrete_override check here *)
        begin
          match child_typeconst.ttc_kind with
          | TCPartiallyAbstract { patc_constraint = c_as; patc_type = c_t } ->
            let env =
              Typing_ops.sub_type_decl
                ~on_error
                pos
                Reason.URsubsume_tconst_cstr
                env
                c_as
                p_as
            in
            Typing_ops.sub_type_decl
              ~on_error
              pos
              Reason.URtypeconst_cstr
              env
              c_t
              p_as
          | TCConcrete { tc_type = c_t } ->
            Typing_ops.sub_type_decl
              ~on_error
              pos
              Reason.URtypeconst_cstr
              env
              c_t
              p_as
          | _ -> env
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
              Errors.typeconst_concrete_concrete_override
                ~current_decl_and_file:(Env.get_current_decl_and_file env)
                pos
                parent_pos
          | _ -> ()
        end;
        env
    in

    (* Don't recheck inherited type constants: errors will
     * have been emitted already for the parent *)
    ( if inherited then
      ()
    else
      match (child_typeconst.ttc_kind, parent_tconst_enforceable) with
      | (TCAbstract { atc_default = Some ty; _ }, (pos, true))
      | ( ( TCPartiallyAbstract { patc_type = ty; _ }
          | TCConcrete { tc_type = ty } ),
          (pos, true) ) ->
        let tast_env = Tast_env.typing_env_as_tast_env env in
        let emit_error =
          Errors.invalid_enforceable_type "constant" (pos, name)
        in
        Typing_enforceable_hint.validate_type
          tast_env
          (fst child_typeconst.ttc_name |> Pos_or_decl.unsafe_to_raw_pos)
          ty
          emit_error
      | _ ->
        ();

        (match parent_typeconst.ttc_reifiable with
        | None -> ()
        | Some pos ->
          let tast_env = Tast_env.typing_env_as_tast_env env in
          Typing_const_reifiable.check_reifiable tast_env child_typeconst pos)
    );

    (* If the parent cannot be overridden, we unify the types otherwise we ensure
     * the child's assigned type is compatible with the parent's
     *
     * TODO(T88552052) restrict concrete typeconst overriding
     *)
    let parent_is_final =
      match parent_typeconst.ttc_kind with
      | TCConcrete _ -> true
      | TCPartiallyAbstract _ ->
        TypecheckerOptions.disable_partially_abstract_typeconsts
          (Env.get_tcopt env)
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
      | TCPartiallyAbstract { patc_type = t; _ }
      | TCConcrete { tc_type = t } ->
        Some t
      | TCAbstract _ -> None
    in
    Option.value ~default:env
    @@ Option.map2
         (opt_type__LEGACY parent_typeconst)
         (opt_type__LEGACY child_typeconst)
         ~f:(check env)

let check_typeconst_override
    env implements class_ parent_tconst tconst parent_class on_error =
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
  | ( (TCConcrete _ | TCPartiallyAbstract _),
      (TCConcrete _ | TCPartiallyAbstract _) )
  | ( TCAbstract { atc_default = Some _; _ },
      (TCConcrete _ | TCPartiallyAbstract _) )
  | ( TCAbstract { atc_default = Some _; _ },
      TCAbstract { atc_default = Some _; _ } ) ->
    if
      (not is_context_constant)
      && (not tconst.ttc_synthesized)
      && (not parent_tconst.ttc_synthesized)
      && conflict_with_declared_interface
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
        | TCAbstract _
        | TCPartiallyAbstract _ ->
          true
      in
      Errors.interface_typeconst_multiple_defs
        pos
        parent_pos
        tconst.ttc_origin
        parent_tconst.ttc_origin
        name
        child_is_abstract
        on_error
  | _ -> ());
  env

(* For type constants we need to check that a child respects the
 * constraints specified by its parent, and does not conflict
 * with other inherited type constants *)
let check_typeconsts env implements parent_class class_ on_error =
  let (parent_pos, parent_class, _) = parent_class in
  let (pos, class_, _) = class_ in
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
        Errors.member_not_implemented
          tconst_name
          parent_pos
          pos
          (fst parent_tconst.ttc_name);
        env)

let check_consts
    env implements parent_class (name_pos, class_) psubst subst on_error =
  let (pconsts, consts) = (Cls.consts parent_class, Cls.consts class_) in
  let pconsts = instantiate_consts psubst pconsts in
  let consts = instantiate_consts subst consts in
  let consts =
    List.fold consts ~init:SMap.empty ~f:(fun m (k, v) -> SMap.add k v m)
  in
  List.fold pconsts ~init:env ~f:(fun env (const_name, parent_const) ->
      if String.( <> ) const_name SN.Members.mClass then (
        match SMap.find_opt const_name consts with
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
              parent_const
              const
              on_error
          | Some _ -> env)
        | None ->
          Errors.member_not_implemented
            const_name
            parent_const.cc_pos
            name_pos
            (Cls.pos parent_class);
          env
      ) else
        env)

(* Use the [on_error] callback if we need to wrap the basic error with a
 *   "Class ... does not correctly implement all required members"
 * message pointing at the class being checked.
 *)
let check_class_implements
    env implements parent_class (name_pos, class_) on_error =
  let get_interfaces acc x =
    let (_, (_, name), _) = TUtils.unwrap_class_type x in
    match Env.get_class env name with
    | Some iface -> iface :: acc
    | None -> acc
  in
  let implements = List.fold ~f:get_interfaces ~init:[] implements in
  let env = check_typeconsts env implements parent_class class_ on_error in
  let (parent_pos, parent_class, parent_tparaml) = parent_class in
  let (pos, class_, tparaml) = class_ in
  let psubst = Inst.make_subst (Cls.tparams parent_class) parent_tparaml in
  let subst = Inst.make_subst (Cls.tparams class_) tparaml in
  let env =
    check_consts
      env
      implements
      parent_class
      (name_pos, class_)
      psubst
      subst
      on_error
  in
  let memberl = make_all_members ~parent_class ~child_class:class_ in
  let env = check_constructors env parent_class class_ psubst subst on_error in
  let check_privates : bool =
    Ast_defs.(equal_class_kind (Cls.kind parent_class) Ctrait)
  in
  if Cls.members_fully_known class_ then
    List.iter memberl (check_members_implemented check_privates parent_pos pos);
  List.fold ~init:env memberl ~f:(fun env ->
      check_members
        check_privates
        env
        (parent_class, psubst)
        (name_pos, class_, subst)
        on_error)

(*****************************************************************************)
(* The externally visible function *)
(*****************************************************************************)

let check_implements env implements parent_type (name_pos, type_to_be_checked) =
  let (_, parent_name, parent_tparaml) = TUtils.unwrap_class_type parent_type in
  let (_, (_, name), tparaml) = TUtils.unwrap_class_type type_to_be_checked in
  let (parent_name_pos, parent_name_str) = parent_name in
  let parent_class = Env.get_class env (snd parent_name) in
  let class_ = Env.get_class env name in
  match (parent_class, class_) with
  | (None, _)
  | (_, None) ->
    env
  | (Some parent_class, Some class_) ->
    let parent_class = (parent_name_pos, parent_class, parent_tparaml) in
    let class_ = (name_pos, class_, tparaml) in
    check_class_implements
      env
      implements
      parent_class
      (name_pos, class_)
      (fun ?code:_ reasons ->
        (* sadly, enum error reporting requires this to keep the error in the file
           with the enum *)
        if String.equal parent_name_str SN.Classes.cHH_BuiltinEnum then
          Errors.bad_enum_decl name_pos reasons
        else
          Errors.bad_decl_override
            parent_name_pos
            parent_name_str
            name_pos
            name
            reasons)
