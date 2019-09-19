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

open Core_kernel
open Typing_defs
module Env = Typing_env
module Dep = Typing_deps.Dep
module TUtils = Typing_utils
module Inst = Decl_instantiate
module Phase = Typing_phase
module SN = Naming_special_names
module Cls = Decl_provider.Class
module MakeType = Typing_make_type
module SubType = Typing_subtype

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let is_private = function
  | { ce_visibility = Vprivate _; _ } -> true
  | _ -> false

let is_lsb = function
  | { ce_lsb = true; _ } -> true
  | _ -> false

(*****************************************************************************)
(* Given a map of members, check that the overriding is correct.
 * Please note that 'members' has a very general meaning here.
 * It can be class variables, methods, static methods etc ... The same logic
 * is applied to verify that the overriding is correct.
 *)
(*****************************************************************************)

let use_parent_for_known = false

(* Rules for visibility *)
let check_visibility parent_vis c_vis parent_pos pos =
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
    Errors.visibility_extends vis pos parent_pos parent_vis

let check_class_elt_visibility parent_class_elt class_elt =
  let parent_vis = parent_class_elt.ce_visibility in
  let c_vis = class_elt.ce_visibility in
  let (lazy (parent_pos, _)) = parent_class_elt.ce_type in
  let (lazy (elt_pos, _)) = class_elt.ce_type in
  let parent_pos = Reason.to_pos parent_pos in
  let pos = Reason.to_pos elt_pos in
  check_visibility parent_vis c_vis parent_pos pos

let check_constant_visibility parent_class_const class_const =
  let (parent_pos, _) = parent_class_const.cc_type in
  let (elt_pos, _) = class_const.cc_type in
  let parent_pos = Reason.to_pos parent_pos in
  let pos = Reason.to_pos elt_pos in
  let parent_vis = parent_class_const.cc_visibility in
  let vis = class_const.cc_visibility in
  check_visibility parent_vis vis parent_pos pos

(* Check that all the required members are implemented *)
let check_members_implemented
    check_private parent_reason reason (_, parent_members, get_member, _) =
  Sequence.iter parent_members (fun (member_name, class_elt) ->
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
        let (lazy (pos, _)) = class_elt.ce_type in
        let defn_reason = Reason.to_pos pos in
        Errors.member_not_implemented
          member_name
          parent_reason
          reason
          defn_reason
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
let check_ambiguous_inheritance f parent child pos class_ origin =
  Errors.try_when
    (f parent child)
    ~when_:(fun () ->
      Cls.name class_ <> origin && Errors.has_no_errors (f child parent))
    ~do_:(fun error ->
      Errors.ambiguous_inheritance pos (Cls.name class_) origin error)

let should_check_params parent_class class_ =
  if use_parent_for_known then
    Cls.members_fully_known parent_class
  else
    Cls.members_fully_known class_

let check_final_method member_source parent_class_elt class_elt =
  (* we only check for final overrides on methods, not properties *)
  let is_method =
    match member_source with
    | `FromMethod
    | `FromSMethod ->
      true
    | _ -> false
  in
  let is_override =
    parent_class_elt.ce_final
    && parent_class_elt.ce_origin <> class_elt.ce_origin
  in
  if is_method && is_override && not class_elt.ce_synthesized then
    (* we have a final method being overridden by a user-declared method *)
    let (lazy (parent_pos, _)) = parent_class_elt.ce_type in
    let (lazy (elt_pos, _)) = class_elt.ce_type in
    let parent_pos = Reason.to_pos parent_pos in
    let pos = Reason.to_pos elt_pos in
    Errors.override_final parent_pos pos

let check_memoizelsb_method member_source parent_class_elt class_elt =
  let is_method =
    match member_source with
    | `FromMethod
    | `FromSMethod ->
      true
    | _ -> false
  in
  let is_memoizelsb = class_elt.ce_memoizelsb in
  let is_override = parent_class_elt.ce_origin <> class_elt.ce_origin in
  if is_method && is_memoizelsb && is_override then
    (* we have a __MemoizeLSB method which is overriding something else *)
    let (lazy (parent_pos, _)) = parent_class_elt.ce_type in
    let (lazy (elt_pos, _)) = class_elt.ce_type in
    let parent_pos = Reason.to_pos parent_pos in
    let pos = Reason.to_pos elt_pos in
    Errors.override_memoizelsb parent_pos pos

let check_lsb_overrides member_source member_name parent_class_elt class_elt =
  let is_sprop =
    match member_source with
    | `FromSProp -> true
    | _ -> false
  in
  let parent_is_lsb = parent_class_elt.ce_lsb in
  let is_override = parent_class_elt.ce_origin <> class_elt.ce_origin in
  if is_sprop && parent_is_lsb && is_override then
    (* __LSB attribute is being overridden *)
    let (lazy (parent_pos, _)) = parent_class_elt.ce_type in
    let (lazy (elt_pos, _)) = class_elt.ce_type in
    let parent_pos = Reason.to_pos parent_pos in
    let pos = Reason.to_pos elt_pos in
    Errors.override_lsb member_name parent_pos pos

let check_lateinit parent_class_elt class_elt =
  let is_override = parent_class_elt.ce_origin <> class_elt.ce_origin in
  let lateinit_diff = parent_class_elt.ce_lateinit <> class_elt.ce_lateinit in
  if is_override && lateinit_diff then
    let (lazy (parent_reason, _)) = parent_class_elt.ce_type in
    let (lazy (child_reason, _)) = class_elt.ce_type in
    let parent_pos = Reason.to_pos parent_reason in
    let child_pos = Reason.to_pos child_reason in
    Errors.bad_lateinit_override
      parent_class_elt.ce_lateinit
      parent_pos
      child_pos

let check_xhp_attr_required env parent_class_elt class_elt =
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
    let parent_attr = parent_class_elt.ce_xhp_attr in
    let attr = class_elt.ce_xhp_attr in
    match (parent_attr, attr) with
    | (Some { xa_tag = parent_tag; _ }, Some { xa_tag = tag; _ })
      when is_less_strict (tag, parent_tag) ->
      let (lazy (parent_reason, _)) = parent_class_elt.ce_type in
      let (lazy (child_reason, _)) = class_elt.ce_type in
      let parent_pos = Reason.to_pos parent_reason in
      let child_pos = Reason.to_pos child_reason in
      let show = function
        | None -> "not required or lateinit"
        | Some Required -> "required"
        | Some Lateinit -> "lateinit"
      in
      Errors.bad_xhp_attr_required_override
        (show parent_tag)
        (show tag)
        parent_pos
        child_pos
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
    class_elt =
  (* We first verify that we aren't overriding a final method *)
  check_final_method mem_source parent_class_elt class_elt;
  check_memoizelsb_method mem_source parent_class_elt class_elt;

  (* Verify that we are not overriding an __LSB property *)
  check_lsb_overrides mem_source member_name parent_class_elt class_elt;
  check_lateinit parent_class_elt class_elt;
  check_xhp_attr_required env parent_class_elt class_elt;
  let check_params = should_check_params parent_class class_ in
  check_class_elt_visibility parent_class_elt class_elt;
  let (lazy fty_child) = class_elt.ce_type in
  let (lazy fty_parent) = parent_class_elt.ce_type in
  let pos = Reason.to_pos (fst fty_child) in
  let parent_pos = Reason.to_pos (fst fty_parent) in
  if class_elt.ce_const <> parent_class_elt.ce_const then
    Errors.overriding_prop_const_mismatch
      parent_pos
      parent_class_elt.ce_const
      pos
      class_elt.ce_const;
  if (not parent_class_elt.ce_abstract) && class_elt.ce_abstract then
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See override_abstract_concrete.php test case for example. *)
    Errors.abstract_concrete_override
      pos
      parent_pos
      ( if mem_source = `FromMethod || mem_source = `FromSMethod then
        `method_
      else
        `property );
  if check_params then
    Errors.try_
      (fun () ->
        match (fty_parent, fty_child) with
        | ((r_parent, Tfun ft_parent), (r_child, Tfun ft_child)) ->
          let is_static = Cls.has_smethod parent_class member_name in
          (* Add deps here when we override *)
          let subtype_funs =
            SubType.subtype_method
              ~extra_info:
                SubType.
                  {
                    method_info = Some (member_name, is_static);
                    class_ty = Some (DeclTy class_ty);
                    parent_class_ty = Some (DeclTy parent_ty);
                  }
              ~check_return:(not ignore_fun_return)
          in
          let check (r1, ft1) (r2, ft2) () =
            ( if check_member_unique then
              match (parent_class_elt.ce_abstract, class_elt.ce_abstract) with
              | (false, false) ->
                (* Multiple concrete trait definitions, error *)
                Errors.multiple_concrete_defs
                  pos
                  parent_pos
                  class_elt.ce_origin
                  parent_class_elt.ce_origin
                  member_name
                  (Cls.name class_)
              | _ -> () );

            (* these unify errors are collected into errorl *)
            subtype_funs env r2 ft2 r1 ft1 Errors.unify_error
          in
          check_ambiguous_inheritance
            check
            (r_parent, ft_parent)
            (r_child, ft_child)
            pos
            class_
            class_elt.ce_origin
        | (fty_parent, _) ->
          (match (snd fty_parent, snd fty_child) with
          | (Tany _, Tany _) -> ()
          | (Tany _, _) -> Errors.decl_override_missing_hint parent_pos
          | (_, Tany _) -> Errors.decl_override_missing_hint pos
          | (_, _) -> ());
          if (not parent_class_elt.ce_abstract) && class_elt.ce_abstract then
            Errors.abstract_concrete_override pos parent_pos `property;
          if class_elt.ce_const then (
            if
              check_member_unique
              && (not class_elt.ce_abstract)
              && not parent_class_elt.ce_abstract
            then
              Errors.multiple_concrete_defs
                pos
                parent_pos
                class_elt.ce_origin
                parent_class_elt.ce_origin
                member_name
                (Cls.name class_);
            Typing_ops.sub_type_decl
              pos
              Typing_reason.URnone
              env
              fty_child
              fty_parent
          ) else
            Typing_ops.unify_decl
              pos
              Typing_reason.URnone
              env
              fty_parent
              fty_child;
          env)
      (fun errorl ->
        Errors.bad_method_override pos member_name errorl;
        env)
  else
    env

let check_const_override
    env const_name parent_class class_ parent_class_const class_const =
  let check_params = should_check_params parent_class class_ in
  let member_not_unique =
    (* Similar to should_check_member_unique, we check if there are multiple
      concrete implementations of class constants with no override.
      (Currently overriding interface constants is not supported in HHVM, but
      we should allow it in Hack files)
    *)
    match Cls.kind parent_class with
    | Ast_defs.Cinterface ->
      (* Synthetic  *)
      (not class_const.cc_synthesized)
      (* The parent we are checking is synthetic, no point in checking *)
      && (not parent_class_const.cc_synthesized)
      (* defined on original class *)
      && class_const.cc_origin <> Cls.name class_
      (* defined from parent class, nothing to check *)
      && class_const.cc_origin <> parent_class_const.cc_origin
      (* Only check if there are multiple concrete definitions *)
      && (not class_const.cc_abstract)
      && not parent_class_const.cc_abstract
    | _ -> false
  in
  check_constant_visibility parent_class_const class_const;
  ( if check_params then
    if member_not_unique then
      Errors.multiple_concrete_defs
        class_const.cc_pos
        parent_class_const.cc_pos
        class_const.cc_origin
        parent_class_const.cc_origin
        const_name
        (Cls.name class_)
    else if (not parent_class_const.cc_abstract) && class_const.cc_abstract
    then
      let pos = Reason.to_pos (fst class_const.cc_type) in
      let parent_pos = Reason.to_pos (fst parent_class_const.cc_type) in
      Errors.abstract_concrete_override pos parent_pos `constant );
  Phase.sub_type_decl
    env
    class_const.cc_type
    parent_class_const.cc_type
    Errors.class_constant_type_mismatch

(* Privates are only visible in the parent, we don't need to check them *)
let filter_privates members =
  Sequence.filter members (fun (_name, class_elt) ->
      (not (is_private class_elt)) || is_lsb class_elt)

let check_members
    check_private
    env
    removals
    (parent_class, psubst, parent_ty)
    (class_, subst, class_ty)
    (mem_source, parent_members, get_member, dep) =
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
      (not class_elt.ce_synthesized)
      (* The parent we are checking is synthetic, no point in checking *)
      && (not parent_class_elt.ce_synthesized)
      (* defined on original class *)
      && class_elt.ce_origin <> Cls.name class_
      (* defined from parent class, nothing to check *)
      && class_elt.ce_origin <> parent_class_elt.ce_origin
    | _ -> false
  in
  Sequence.fold
    ~init:env
    parent_members
    ~f:(fun env (member_name, parent_class_elt) ->
      (* for this particular member, check to see if the parent considered
      is a trait that was removed, otherwise subtype with the declaration in
      that parent *)
      let removed =
        match String.Map.find removals member_name with
        | Some traits ->
          List.mem traits parent_class_elt.ce_origin ~equal:String.equal
        | None -> false
      in
      if not removed then
        match get_member member_name with
        | Some class_elt ->
          let parent_class_elt = Inst.instantiate_ce psubst parent_class_elt in
          let class_elt = Inst.instantiate_ce subst class_elt in
          let check_member_unique =
            should_check_member_unique class_elt parent_class_elt
          in
          if parent_class_elt.ce_origin <> class_elt.ce_origin then
            Typing_deps.add_idep
              (Dep.Class (Cls.name class_))
              (dep parent_class_elt.ce_origin member_name);
          check_override
            ~check_member_unique
            env
            member_name
            mem_source
            (parent_class, parent_ty)
            (class_, class_ty)
            parent_class_elt
            class_elt
        | None -> env
      else
        env)

(*****************************************************************************)
(* Before checking that a class implements an interface, we have to
 * substitute the type parameters with their real type.
 *)
(*****************************************************************************)

(* Instantiation basically applies the substitution *)
let instantiate_consts subst consts =
  Sequence.map consts (fun (id, cc) -> (id, Inst.instantiate_cc subst cc))

let make_all_members ~child_class ~parent_class =
  [
    ( `FromProp,
      Cls.props parent_class,
      Cls.get_prop child_class,
      (fun x y -> Dep.Prop (x, y)) );
    ( `FromSProp,
      Cls.sprops parent_class,
      Cls.get_sprop child_class,
      (fun x y -> Dep.SProp (x, y)) );
    ( `FromMethod,
      Cls.methods parent_class,
      Cls.get_method child_class,
      (fun x y -> Dep.Method (x, y)) );
    ( `FromSMethod,
      Cls.smethods parent_class,
      Cls.get_smethod child_class,
      (fun x y -> Dep.SMethod (x, y)) );
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
      ft_pos = pos;
      ft_deprecated = None;
      ft_is_coroutine = false;
      ft_arity = Fstandard (0, 0);
      ft_tparams = ([], FTKtparams);
      ft_where_constraints = [];
      ft_params = [];
      ft_ret = { et_type = MakeType.void r; et_enforced = false };
      ft_fun_kind = Ast_defs.FSync;
      ft_reactive = Nonreactive;
      ft_mutability = None;
      ft_returns_mutable = false;
      ft_return_disposable = false;
      ft_decl_errors = None;
      ft_returns_void_to_rx = false;
    }
  in
  {
    ce_abstract = false;
    ce_final = false;
    ce_xhp_attr = None;
    ce_const = false;
    ce_lateinit = false;
    ce_override = false;
    ce_lsb = false;
    ce_memoizelsb = false;
    ce_synthesized = true;
    ce_visibility = Vpublic;
    ce_type = lazy (r, Tfun ft);
    ce_origin = name;
  }

(* When an interface defines a constructor, we check that they are compatible *)
let check_constructors
    env (parent_class, parent_ty) (class_, class_ty) psubst subst =
  let consistent = snd (Cls.construct parent_class) <> Inconsistent in
  if Cls.kind parent_class = Ast_defs.Cinterface || consistent then
    match (fst (Cls.construct parent_class), fst (Cls.construct class_)) with
    | (Some parent_cstr, _) when parent_cstr.ce_synthesized -> env
    | (Some parent_cstr, None) ->
      let (lazy (pos, _)) = parent_cstr.ce_type in
      Errors.missing_constructor (Reason.to_pos pos);
      env
    | (_, Some cstr) when cstr.ce_override ->
      (* <<__UNSAFE_Construct>> *)
      env
    | (Some parent_cstr, Some cstr) ->
      let parent_cstr = Inst.instantiate_ce psubst parent_cstr in
      let cstr = Inst.instantiate_ce subst cstr in
      if parent_cstr.ce_origin <> cstr.ce_origin then
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
    | (None, Some cstr) when consistent ->
      let parent_cstr = default_constructor_ce parent_class in
      let parent_cstr = Inst.instantiate_ce psubst parent_cstr in
      let cstr = Inst.instantiate_ce subst cstr in
      if parent_cstr.ce_origin <> cstr.ce_origin then
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
    | (None, _) -> env
  else (
    begin
      match (fst (Cls.construct parent_class), fst (Cls.construct class_)) with
      | (Some parent_cstr, _) when parent_cstr.ce_synthesized -> ()
      | (Some parent_cstr, Some child_cstr) ->
        check_final_method `FromMethod parent_cstr child_cstr;
        check_class_elt_visibility parent_cstr child_cstr
      | (_, _) -> ()
    end;
    env
  )

(* Checks if a child is compatible with the type constant of its parent.
 * This requires the child's constraint and assigned type to be a subtype of
 * the parent's type constant.
 *)
let tconst_subsumption env parent_typeconst child_typeconst =
  let (pos, name) = child_typeconst.ttc_name in
  let parent_pos = fst parent_typeconst.ttc_name in
  let parent_is_concrete = Option.is_some parent_typeconst.ttc_type in
  let disable_partially_abstract =
    TypecheckerOptions.disable_partially_abstract_typeconsts
      (Env.get_tcopt env)
  in
  let is_final =
    match parent_typeconst.ttc_abstract with
    | TCPartiallyAbstract
    | TCConcrete
      when disable_partially_abstract ->
      true
    | _ -> parent_is_concrete && Option.is_none parent_typeconst.ttc_constraint
  in
  match (parent_typeconst.ttc_abstract, child_typeconst.ttc_abstract) with
  | (TCAbstract (Some _), TCAbstract None) ->
    Errors.override_no_default_typeconst pos parent_pos
  | _ ->
    ();

    (* Check that the child's constraint is compatible with the parent. If the
     * parent has a constraint then the child must also have a constraint if it
     * is abstract
     *)
    let child_is_abstract = Option.is_none child_typeconst.ttc_type in
    if parent_is_concrete && child_is_abstract then
      (* It is valid for abstract class to extend a concrete class, but it cannot
       * redefine already concrete members as abstract.
       * See typecheck/tconst/subsume_tconst5.php test case for example. *)
      Errors.abstract_concrete_override pos parent_pos `typeconst;

    let default =
      (Reason.Rtconst_no_cstr child_typeconst.ttc_name, Tgeneric name)
    in
    let child_cstr =
      if child_is_abstract then
        Some (Option.value child_typeconst.ttc_constraint ~default)
      else
        child_typeconst.ttc_constraint
    in
    ignore
    @@ Option.map2
         child_cstr
         parent_typeconst.ttc_constraint
         ~f:(Typing_ops.sub_type_decl pos Reason.URsubsume_tconst_cstr env);

    (* Check that the child's assigned type satisifies parent constraint *)
    ignore
    @@ Option.map2
         child_typeconst.ttc_type
         parent_typeconst.ttc_constraint
         ~f:(Typing_ops.sub_type_decl parent_pos Reason.URtypeconst_cstr env);

    begin
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
        Enforceable_hint_check.validator#validate_type tast_env ty emit_error
      | _ -> ()
    end;

    begin
      match parent_typeconst.ttc_reifiable with
      | None -> ()
      | Some pos ->
        let tast_env = Tast_env.typing_env_as_tast_env env in
        Type_const_check.check_reifiable tast_env child_typeconst pos
    end;

    (* If the parent cannot be overridden, we unify the types otherwise we ensure
     * the child's assigned type is compatible with the parent's *)
    let check x y =
      if is_final then
        Typing_ops.unify_decl pos Reason.URsubsume_tconst_assign env x y
      else
        Typing_ops.sub_type_decl pos Reason.URsubsume_tconst_assign env y x
    in
    ignore
    @@ Option.map2 parent_typeconst.ttc_type child_typeconst.ttc_type ~f:check

(* For type constants we need to check that a child respects the
 * constraints specified by its parent. *)
let check_typeconsts env parent_class class_ =
  let (parent_pos, parent_class, _) = parent_class in
  let (pos, class_, _) = class_ in
  let ptypeconsts = Cls.typeconsts parent_class in
  let tconst_check parent_tconst tconst () =
    tconst_subsumption env parent_tconst tconst
  in
  Sequence.iter ptypeconsts (fun (tconst_name, parent_tconst) ->
      match Cls.get_typeconst class_ tconst_name with
      | Some tconst ->
        check_visibility
          parent_tconst.ttc_visibility
          tconst.ttc_visibility
          (fst parent_tconst.ttc_name)
          (fst tconst.ttc_name);
        check_ambiguous_inheritance
          tconst_check
          parent_tconst
          tconst
          (fst tconst.ttc_name)
          class_
          tconst.ttc_origin
      | None ->
        Errors.member_not_implemented
          tconst_name
          parent_pos
          pos
          (fst parent_tconst.ttc_name))

let check_consts env parent_class class_ psubst subst =
  let (pconsts, consts) = (Cls.consts parent_class, Cls.consts class_) in
  let pconsts = instantiate_consts psubst pconsts in
  let consts = instantiate_consts subst consts in
  let consts =
    Sequence.fold consts ~init:SMap.empty ~f:(fun m (k, v) -> SMap.add k v m)
  in
  Sequence.iter pconsts (fun (const_name, parent_const) ->
      if const_name <> SN.Members.mClass then
        match SMap.get const_name consts with
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
          | Some _ -> ())
        | None ->
          let parent_pos = Reason.to_pos (fst parent_const.cc_type) in
          Errors.member_not_implemented
            const_name
            parent_pos
            (Cls.pos class_)
            (Cls.pos parent_class));
  ()

let check_class_implements
    env removals (parent_class, parent_ty) (class_, class_ty) =
  check_typeconsts env parent_class class_;
  let (parent_pos, parent_class, parent_tparaml) = parent_class in
  let (pos, class_, tparaml) = class_ in
  let fully_known = Cls.members_fully_known class_ in
  let psubst = Inst.make_subst (Cls.tparams parent_class) parent_tparaml in
  let subst = Inst.make_subst (Cls.tparams class_) tparaml in
  check_consts env parent_class class_ psubst subst;
  let memberl = make_all_members ~parent_class ~child_class:class_ in
  let env =
    check_constructors
      env
      (parent_class, parent_ty)
      (class_, class_ty)
      psubst
      subst
  in
  let check_privates : bool = Cls.kind parent_class = Ast_defs.Ctrait in
  if not fully_known then
    ()
  else
    List.iter memberl (check_members_implemented check_privates parent_pos pos);
  List.fold ~init:env memberl ~f:(fun env ->
      check_members
        check_privates
        env
        removals
        (parent_class, psubst, parent_ty)
        (class_, subst, class_ty))

(*****************************************************************************)
(* The externally visible function *)
(*****************************************************************************)

let check_implements env removals parent_type type_ =
  let (parent_r, parent_name, parent_tparaml) =
    TUtils.unwrap_class_type parent_type
  in
  let (r, name, tparaml) = TUtils.unwrap_class_type type_ in
  let parent_class = Env.get_class env (snd parent_name) in
  let class_ = Env.get_class env (snd name) in
  match (parent_class, class_) with
  | (None, _)
  | (_, None) ->
    env
  | (Some parent_class, Some class_) ->
    let parent_class =
      (Reason.to_pos parent_r, parent_class, parent_tparaml)
    in
    let class_ = (Reason.to_pos r, class_, tparaml) in
    if snd parent_name = SN.Classes.cHH_BuiltinEnum then
      (* sadly, enum error reporting requires this to keep the error in the file
               with the enum *)
      Errors.try_
        (fun () ->
          check_class_implements
            env
            removals
            (parent_class, parent_type)
            (class_, type_))
        (fun errorl ->
          let (name_pos, _) = name in
          Errors.bad_enum_decl name_pos errorl;
          env)
    else
      Errors.try_unless_error_in_different_file
        (Env.get_file env)
        (fun () ->
          check_class_implements
            env
            removals
            (parent_class, parent_type)
            (class_, type_))
        (fun errorl ->
          let (p_name_pos, p_name_str) = parent_name in
          let (name_pos, name_str) = name in
          Errors.bad_decl_override
            p_name_pos
            p_name_str
            name_pos
            name_str
            errorl)
