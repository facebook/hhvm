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
    Errors.override_final
      ~parent:parent_pos
      ~child:pos
      ~on_error:(Some on_error)

let check_memoizelsb_method parent_class_elt class_elt on_error =
  if get_ce_memoizelsb class_elt then
    (* we have a __MemoizeLSB method which is overriding something else *)
    let (lazy parent_pos) = parent_class_elt.ce_pos in
    let (lazy pos) = class_elt.ce_pos in
    Errors.override_memoizelsb parent_pos pos on_error

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
    (parent_class, parent_ty)
    (class_, class_ty)
    parent_class_elt
    class_elt
    on_error =
  (* If the class element is defined in the class that we're checking, then
   * don't wrap with the extra
   * "Class ... does not correctly implement all required members" message *)
  let on_error =
    if String.equal class_elt.ce_origin (Cls.name class_) then
      Errors.unify_error
    else
      on_error
  in
  if is_method mem_source then begin
    (* We first verify that we aren't overriding a final method *)
    (* we only check for final overrides on methods, not properties *)
    (* we don't check constructors, as they are already checked
     * in the decl phase *)
    check_final_method parent_class_elt class_elt on_error;
    check_memoizelsb_method parent_class_elt class_elt on_error;
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
        `property );
  if check_params then (
    let on_error ?code:_ errorl =
      ( if is_method then
        Errors.bad_method_override
      else
        Errors.bad_prop_override )
        pos
        member_name
        errorl
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
      let is_static = Cls.has_smethod parent_class member_name in
      let check (r1, ft1) (r2, ft2) () =
        match mem_source with
        | `FromConstructor ->
          (* we don't check that constructor signatures follow
           * subtyping rules except with __ConsistentConstruct,
           * which is checked elsewhere *)
          env
        | _ ->
          Typing_subtype_method.(
            (* Add deps here when we override *)
            subtype_method_decl
              ~extra_info:
                Typing_subtype.
                  {
                    method_info = Some (member_name, is_static);
                    class_ty = Some (DeclTy class_ty);
                    parent_class_ty = Some (DeclTy parent_ty);
                  }
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
        (r_parent, ft_parent)
        (r_child, ft_child)
        pos
        class_
        class_elt.ce_origin
        on_error
    | _ ->
      if get_ce_const class_elt then
        Typing_ops.sub_type_decl_on_error
          pos
          Typing_reason.URnone
          env
          on_error
          fty_child
          fty_parent
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

let check_const_override
    env const_name parent_class class_ parent_class_const class_const on_error =
  let check_params = should_check_params parent_class class_ in

  (* Shared preconditons for member_not_unique and is_bad_interface_const_override *)
  let interface_pre_checks =
    match Cls.kind parent_class with
    | Ast_defs.Cinterface ->
      (* Synthetic  *)
      (not class_const.cc_synthesized)
      (* The parent we are checking is synthetic, no point in checking *)
      && (not parent_class_const.cc_synthesized)
      (* Only check if parent and child have concrete definitions *)
      && (not class_const.cc_abstract)
      && not parent_class_const.cc_abstract
    | _ -> false
  in
  let member_not_unique =
    (* Similar to should_check_member_unique, we check if there are multiple
      concrete implementations of class constants with no override.
    *)
    interface_pre_checks
    (* defined on original class *)
    && String.( <> ) class_const.cc_origin (Cls.name class_)
    (* defined from parent class, nothing to check *)
    && String.( <> ) class_const.cc_origin parent_class_const.cc_origin
  in
  let is_bad_interface_const_override =
    (* HHVM does not support one specific case of overriding constants:
     If the original constant was defined as non-abstract in an interface,
     it cannot be overridden when implementing or extending that interface. *)

    (* Check that the constant is indeed defined in class_ *)
    interface_pre_checks && String.( = ) class_const.cc_origin (Cls.name class_)
  in
  let is_abstract_concrete_override =
    (not parent_class_const.cc_abstract) && class_const.cc_abstract
  in

  if check_params then
    if member_not_unique then
      Errors.multiple_concrete_defs
        class_const.cc_pos
        parent_class_const.cc_pos
        class_const.cc_origin
        parent_class_const.cc_origin
        const_name
        (Cls.name class_)
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
        `constant;
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
    (parent_class, psubst, parent_ty)
    (class_, subst, class_ty)
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
    of a class member. This means that if the parent class is a trait or interface,
    we need to check that the child member is *uniquely inherited*.

    A member is uniquely inherited if any of the following hold:
    1. It is synthetic (from a requirement)
    2. It is defined on the child class
    3. It is concretely defined in exactly one place
    4. It is abstract, and all other declarations are identical
    *)
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
        Typing_deps.add_idep (Dep.Class (Cls.name class_)) dep;
        check_override
          ~check_member_unique
          env
          member_name
          mem_source
          (parent_class, parent_ty)
          (class_, class_ty)
          parent_class_elt
          class_elt
          on_error
      | _ -> env)

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
  let r = Reason.Rwitness pos in
  (* reason doesn't get used in, e.g. arity checks *)
  let ft =
    {
      ft_arity = Fstandard;
      ft_tparams = [];
      ft_where_constraints = [];
      ft_params = [];
      ft_implicit_params = { capability = MakeType.default_capability r };
      (* TODO(coeffects) relate constructor caps to class caps? *)
      ft_ret = { et_type = MakeType.void r; et_enforced = false };
      ft_flags = 0;
      ft_reactive = Nonreactive;
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
        ~memoizelsb:false
        ~synthesized:true
        ~dynamicallycallable:false;
  }

(* When an interface defines a constructor, we check that they are compatible *)
let check_constructors
    env (parent_class, parent_ty) (class_, class_ty) psubst subst on_error =
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
        Typing_deps.add_idep
          (Dep.Class (Cls.name class_))
          (Dep.Cstr parent_cstr.ce_origin);
        check_override
          env
          ~check_member_unique:false
          "__construct"
          `FromMethod
          ~ignore_fun_return:true
          (parent_class, parent_ty)
          (class_, class_ty)
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
let tconst_subsumption env class_name parent_typeconst child_typeconst on_error
    =
  let (pos, name) = child_typeconst.ttc_name in
  let parent_pos = fst parent_typeconst.ttc_name in
  match (parent_typeconst.ttc_abstract, child_typeconst.ttc_abstract) with
  | (TCAbstract (Some _), TCAbstract None) ->
    Errors.override_no_default_typeconst pos parent_pos;
    env
  | ((TCConcrete | TCPartiallyAbstract), TCAbstract _) ->
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See typecheck/tconst/subsume_tconst5.php test case for example. *)
    Errors.abstract_concrete_override pos parent_pos `typeconst;
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
        Errors.unify_error
    in

    (* Check that the child's constraint is compatible with the parent. If the
     * parent has a constraint then the child must also have a constraint if it
     * is abstract
     *)
    let default =
      MakeType.generic (Reason.Rtconst_no_cstr child_typeconst.ttc_name) name
    in
    let child_cstr =
      match child_typeconst.ttc_abstract with
      | TCAbstract _ ->
        Some (Option.value child_typeconst.ttc_constraint ~default)
      | _ -> child_typeconst.ttc_constraint
    in
    let env =
      Option.value ~default:env
      @@ Option.map2
           child_cstr
           parent_typeconst.ttc_constraint
           ~f:
             (Typing_ops.sub_type_decl_on_error
                pos
                Reason.URsubsume_tconst_cstr
                env
                on_error)
    in

    (* Check that the child's assigned type satisifies parent constraint *)
    let env =
      Option.value ~default:env
      @@ Option.map2
           child_typeconst.ttc_type
           parent_typeconst.ttc_constraint
           ~f:
             (Typing_ops.sub_type_decl_on_error
                pos
                Reason.URtypeconst_cstr
                env
                on_error)
    in

    (* Don't recheck inherited type constants: errors will
     * have been emitted already for the parent *)
    ( if inherited then
      ()
    else
      match
        ( child_typeconst.ttc_abstract,
          child_typeconst.ttc_type,
          parent_typeconst.ttc_enforceable )
      with
      | (TCAbstract (Some ty), _, (pos, true))
      | ((TCPartiallyAbstract | TCConcrete), Some ty, (pos, true)) ->
        let tast_env = Tast_env.typing_env_as_tast_env env in
        let emit_error =
          Errors.invalid_enforceable_type "constant" (pos, name)
        in
        Enforceable_hint_check.validator#validate_type
          tast_env
          (fst child_typeconst.ttc_name)
          ty
          emit_error
      | _ ->
        ();

        (match parent_typeconst.ttc_reifiable with
        | None -> ()
        | Some pos ->
          let tast_env = Tast_env.typing_env_as_tast_env env in
          Type_const_check.check_reifiable tast_env child_typeconst pos) );

    (* If the parent cannot be overridden, we unify the types otherwise we ensure
     * the child's assigned type is compatible with the parent's *)
    let parent_is_final =
      match parent_typeconst.ttc_abstract with
      | TCConcrete -> true
      | TCPartiallyAbstract ->
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
        Typing_ops.sub_type_decl_on_error
          pos
          Reason.URsubsume_tconst_assign
          env
          on_error
          y
          x
    in
    Option.value ~default:env
    @@ Option.map2
         parent_typeconst.ttc_type
         child_typeconst.ttc_type
         ~f:(check env)

(* For type constants we need to check that a child respects the
 * constraints specified by its parent. *)
let check_typeconsts env parent_class class_ on_error =
  let (parent_pos, parent_class, _) = parent_class in
  let (pos, class_, _) = class_ in
  let ptypeconsts = Cls.typeconsts parent_class in
  let tconst_check parent_tconst tconst () =
    tconst_subsumption env (Cls.name class_) parent_tconst tconst on_error
  in
  List.fold ptypeconsts ~init:env ~f:(fun env (tconst_name, parent_tconst) ->
      match Cls.get_typeconst class_ tconst_name with
      | Some tconst ->
        check_ambiguous_inheritance
          tconst_check
          parent_tconst
          tconst
          (fst tconst.ttc_name)
          class_
          tconst.ttc_origin
          on_error
      | None ->
        Errors.member_not_implemented
          tconst_name
          parent_pos
          pos
          (fst parent_tconst.ttc_name);
        env)

let check_consts env parent_class class_ psubst subst on_error =
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
            (Cls.pos class_)
            (Cls.pos parent_class);
          env
      ) else
        env)

(* Use the [on_error] callback if we need to wrap the basic error with a
 *   "Class ... does not correctly implement all required members"
 * message pointing at the class being checked.
 *)
let check_class_implements
    env (parent_class, parent_ty) (class_, class_ty) on_error =
  let env = check_typeconsts env parent_class class_ on_error in
  let (parent_pos, parent_class, parent_tparaml) = parent_class in
  let (pos, class_, tparaml) = class_ in
  let fully_known = Cls.members_fully_known class_ in
  let psubst = Inst.make_subst (Cls.tparams parent_class) parent_tparaml in
  let subst = Inst.make_subst (Cls.tparams class_) tparaml in
  let env = check_consts env parent_class class_ psubst subst on_error in
  let memberl = make_all_members ~parent_class ~child_class:class_ in
  let env =
    check_constructors
      env
      (parent_class, parent_ty)
      (class_, class_ty)
      psubst
      subst
      on_error
  in
  let check_privates : bool =
    Ast_defs.(equal_class_kind (Cls.kind parent_class) Ctrait)
  in
  if fully_known then
    List.iter memberl (check_members_implemented check_privates parent_pos pos);
  List.fold ~init:env memberl ~f:(fun env ->
      check_members
        check_privates
        env
        (parent_class, psubst, parent_ty)
        (class_, subst, class_ty)
        on_error)

(*****************************************************************************)
(* The externally visible function *)
(*****************************************************************************)

let check_implements env parent_type type_to_be_checked =
  let (_, parent_name, parent_tparaml) = TUtils.unwrap_class_type parent_type in
  let (_, name, tparaml) = TUtils.unwrap_class_type type_to_be_checked in
  let (name_pos, name_str) = name in
  let (parent_name_pos, parent_name_str) = parent_name in
  let parent_class = Env.get_class env (snd parent_name) in
  let class_ = Env.get_class env (snd name) in
  match (parent_class, class_) with
  | (None, _)
  | (_, None) ->
    env
  | (Some parent_class, Some class_) ->
    let parent_class = (parent_name_pos, parent_class, parent_tparaml) in
    let class_ = (name_pos, class_, tparaml) in
    check_class_implements
      env
      (parent_class, parent_type)
      (class_, type_to_be_checked)
      (fun ?code:_ errorl ->
        (* sadly, enum error reporting requires this to keep the error in the file
           with the enum *)
        if String.equal parent_name_str SN.Classes.cHH_BuiltinEnum then
          Errors.bad_enum_decl name_pos errorl
        else
          Errors.bad_decl_override
            parent_name_pos
            parent_name_str
            name_pos
            name_str
            errorl)
