(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module dealing with inheritance.
 * When we want to declare a new class, we first have to retrieve all the
 * types that were inherited from their parents.
 *)
(*****************************************************************************)

open Hh_prelude
open Option.Monad_infix
open Decl_defs
open Shallow_decl_defs
open Typing_defs
module Inst = Decl_instantiate

(*****************************************************************************)
(* This is what we are trying to produce for a given class. *)
(*****************************************************************************)

type inherited = {
  ih_substs: subst_context SMap.t;
  ih_cstr: (Decl_defs.element * fun_elt option) option * consistent_kind;
  ih_consts: class_const SMap.t;
  ih_typeconsts: typeconst_type SMap.t;
  ih_props: (Decl_defs.element * decl_ty option) SMap.t;
  ih_sprops: (Decl_defs.element * decl_ty option) SMap.t;
  ih_methods: (Decl_defs.element * fun_elt option) SMap.t;
  ih_smethods: (Decl_defs.element * fun_elt option) SMap.t;
  ih_support_dynamic_type: bool;
}

let empty =
  {
    ih_substs = SMap.empty;
    ih_cstr = (None, Inconsistent);
    ih_consts = SMap.empty;
    ih_typeconsts = SMap.empty;
    ih_props = SMap.empty;
    ih_sprops = SMap.empty;
    ih_methods = SMap.empty;
    ih_smethods = SMap.empty;
    ih_support_dynamic_type = false;
  }

(*****************************************************************************)
(* Functions used to merge an additional inherited class to the types
 * we already inherited.
 *)
(*****************************************************************************)

module type Member_S = sig
  type t

  val is_abstract : t -> bool

  val is_synthesized : t -> bool

  val has_lsb : t -> bool

  val visibility : t -> ce_visibility
end

module Decl_defs_element : Member_S with type t = Decl_defs.element = struct
  type t = Decl_defs.element

  let is_abstract = get_elt_abstract

  let is_synthesized = get_elt_synthesized

  let has_lsb = get_elt_lsb

  let visibility m = m.elt_visibility
end

module Typing_defs_class_elt : Member_S with type t = Typing_defs.class_elt =
struct
  type t = Typing_defs.class_elt

  let is_abstract = get_ce_abstract

  let is_synthesized = get_ce_synthesized

  let has_lsb = get_ce_lsb

  let visibility m = m.ce_visibility
end

module OverridePrecedence : sig
  (** The override precedence of a member is used to determine if a member overrides
    previous members from other parents with the same name. *)
  type t

  val make : (module Member_S with type t = 'member) -> 'member -> t

  val ( > ) : t -> t -> bool

  val is_highest : t -> bool
end = struct
  type t = {
    is_concrete: bool;
    is_not_synthesized: bool;
  }

  let make
      (type member)
      (module Member : Member_S with type t = member)
      (member : member) =
    {
      is_concrete = not @@ Member.is_abstract member;
      is_not_synthesized = not @@ Member.is_synthesized member;
    }

  let bool_to_int b =
    if b then
      1
    else
      0

  let to_int { is_concrete; is_not_synthesized } =
    (2 * bool_to_int is_concrete) + bool_to_int is_not_synthesized

  let ( > ) x y = Int.( > ) (to_int x) (to_int y)

  let is_highest { is_concrete; is_not_synthesized } =
    is_concrete && is_not_synthesized
end

(** Reasons to keep the old signature:
  - We don't want to override a concrete method with
    an abstract one.
  - We don't want to override a method that's actually
    implemented by the programmer with one that's "synthetic",
    e.g. arising merely from a require-extends declaration in
    a trait.
When these two considerations conflict, we give precedence to
abstractness for determining priority of the method.
It's possible to implement this boolean logic by just comparing
the boolean tuples (is_concrete, is_non_synthesized) of each member,
which we do in the OverridePrecedence module. *)
let should_keep_old_sig
    ((sig_, _) : Decl_defs.element * fun_elt option)
    ((old_sig, _) : Decl_defs.element * fun_elt option) : bool =
  let precedence = OverridePrecedence.make (module Decl_defs_element) in
  OverridePrecedence.(precedence old_sig > precedence sig_)

let get_updated_sort_text new_sig old_sig =
  let (sig_decl_element, _) = new_sig in
  let (old_sig_decl_element, _) = old_sig in
  match
    (sig_decl_element.elt_sort_text, old_sig_decl_element.elt_sort_text)
  with
  | (None, Some _text) -> old_sig_decl_element.elt_sort_text
  | _ -> sig_decl_element.elt_sort_text

let add_method name sig_ methods =
  match SMap.find_opt name methods with
  | None ->
    (* The method didn't exist so far, let's add it *)
    SMap.add name sig_ methods
  | Some old_sig ->
    let elt_sort_text = get_updated_sort_text sig_ old_sig in
    if should_keep_old_sig sig_ old_sig then
      let (old_sig_decl_element, old_sig_fun_element) = old_sig in
      SMap.add
        name
        ({ old_sig_decl_element with elt_sort_text }, old_sig_fun_element)
        methods
    (* Otherwise, we *are* overwriting a method definition. This is
     * OK when a naming conflict is parent class vs trait (trait
     * wins!), but not really OK when the naming conflict is trait vs
     * trait (we rely on HHVM to catch the error at runtime) *)
    else
      let sig_ = Tuple.T2.map_fst sig_ ~f:reset_elt_superfluous_override in
      let (sig_decl_element, sig_fun_element) = sig_ in
      SMap.add
        name
        ({ sig_decl_element with elt_sort_text }, sig_fun_element)
        methods
(* take old and new sig, get sort texts recheck *)

let add_methods methods' acc = SMap.fold add_method methods' acc

let add_const name const acc =
  match SMap.find_opt name acc with
  | None -> SMap.add name const acc
  | Some existing_const ->
    (match
       ( const.cc_synthesized,
         existing_const.cc_synthesized,
         const.cc_abstract,
         existing_const.cc_abstract )
     with
    | (true, false, _, _) ->
      (* Don't replace a constant with a synthesized constant. This
         covers the following case:

         class HasFoo { abstract const int FOO; }
         trait T { require extends Foo; }
         class Child extends HasFoo {
            use T;
         }

         In this case, Child still doesn't have a value for the FOO
         constant. *)
      acc
    | (_, _, CCAbstract false, CCAbstract true)
    | (_, _, CCAbstract _, CCConcrete) ->
      (* Don't replace a concrete constant with an abstract constant
           found later in the MRO.*)
      acc
    | (_, _, _, _) -> SMap.add name const acc)

let add_members members acc = SMap.fold SMap.add members acc

let add_typeconst env c name sig_ typeconsts =
  let fix_synthesized =
    TypecheckerOptions.enable_strict_const_semantics (Decl_env.tcopt env) > 3
  in
  match SMap.find_opt name typeconsts with
  | None ->
    (* The type constant didn't exist so far, let's add it *)
    SMap.add name sig_ typeconsts
  | Some old_sig ->
    let typeconsts =
      (* Boolean OR for the second element of the tuple. If some typeconst in
         some ancestor was enforceable, then the child class' typeconst will be
         enforceable too, even if we didn't take that ancestor typeconst. *)
      if snd sig_.ttc_enforceable && not (snd old_sig.ttc_enforceable) then
        SMap.add
          name
          { old_sig with ttc_enforceable = sig_.ttc_enforceable }
          typeconsts
      else
        typeconsts
    in
    (match
       ( old_sig.ttc_synthesized,
         sig_.ttc_synthesized,
         old_sig.ttc_kind,
         sig_.ttc_kind )
     with
    | (false, true, _, _) when Ast_defs.is_c_class c.sc_kind && fix_synthesized
      ->
      (* Don't replace a type constant with a synthesized type constant. This
         covers the following case:

         class A { const type T = int; }
         trait T { require extends A; }
         class Child extends A {
            use T;
         }

         Child should not consider T to be synthesized. *)
      typeconsts
    (* This covers the following case
     *
     * interface I1 { abstract const type T; }
     * interface I2 { const type T = int; }
     *
     * class C implements I1, I2 {}
     *
     * Then C::T == I2::T since I2::T is not abstract
     *)
    | (_, _, TCConcrete _, TCAbstract _) -> typeconsts
    (* This covers the following case
     *
     * interface I {
     *   abstract const type T as arraykey;
     * }
     *
     * abstract class A {
     *   abstract const type T as arraykey = string;
     * }
     *
     * final class C extends A implements I {}
     *
     * C::T must come from A, not I, as A provides the default that will synthesize
     * into a concrete type constant in C.
     *)
    | ( _,
        _,
        TCAbstract { atc_default = Some _; _ },
        TCAbstract { atc_default = None; _ } ) ->
      typeconsts
    | (_, _, _, _) ->
      (* When a type constant is declared in multiple parents we need to make a
       * subtle choice of what type we inherit. For example in:
       *
       * interface I1 { abstract const type t as Container<int>; }
       * interface I2 { abstract const type t as KeyedContainer<int, int>; }
       * abstract class C implements I1, I2 {}
       *
       * Depending on the order the interfaces are declared, we may report an error.
       * Since this could be confusing there is special logic in Typing_extends that
       * checks for this potentially ambiguous situation and warns the programmer to
       * explicitly declare T in C.
       *)
      let sig_ =
        (* Boolean OR for the second element of the tuple. If a typeconst we
           already inherited from some other ancestor was enforceable, then the
           one we inherit here will be enforceable too. *)
        if snd old_sig.ttc_enforceable && not (snd sig_.ttc_enforceable) then
          { sig_ with ttc_enforceable = old_sig.ttc_enforceable }
        else
          sig_
      in
      SMap.add name sig_ typeconsts)

let add_constructor (cstr, cstr_consist) (acc, acc_consist) =
  let ce =
    match (cstr, acc) with
    | (None, _) -> acc
    | (Some ce, Some acce) when should_keep_old_sig ce acce -> acc
    | _ -> cstr
  in
  (ce, Decl_utils.coalesce_consistent acc_consist cstr_consist)

let add_inherited env c inherited acc =
  {
    ih_substs =
      SMap.merge
        begin
          fun _ old_subst_opt new_subst_opt ->
            match (old_subst_opt, new_subst_opt) with
            | (None, None) -> None
            | (Some s, None)
            | (None, Some s) ->
              Some s
            (* If the old subst_context came via require extends, then we want to use
             * the substitutions from the actual extends instead. e.g.,
             *
             * class Base<+T> {}
             * trait MyTrait { require extends Base<mixed>; }
             * class Child extends Base<int> { use MyTrait; }
             *
             * Here the subst_context (MyTrait/[T -> mixed]) should be overridden by
             * (Child/[T -> int]), because it's the actual extension of class Base.
             *)
            | (Some old_subst, Some new_subst) ->
              if
                (not new_subst.sc_from_req_extends)
                || old_subst.sc_from_req_extends
              then
                Some new_subst
              else
                Some old_subst
        end
        acc.ih_substs
        inherited.ih_substs;
    ih_cstr = add_constructor inherited.ih_cstr acc.ih_cstr;
    ih_consts = SMap.fold add_const inherited.ih_consts acc.ih_consts;
    ih_typeconsts =
      SMap.fold (add_typeconst env c) inherited.ih_typeconsts acc.ih_typeconsts;
    ih_props = add_members inherited.ih_props acc.ih_props;
    ih_sprops = add_members inherited.ih_sprops acc.ih_sprops;
    ih_methods = add_methods inherited.ih_methods acc.ih_methods;
    ih_smethods = add_methods inherited.ih_smethods acc.ih_smethods;
    ih_support_dynamic_type =
      inherited.ih_support_dynamic_type || acc.ih_support_dynamic_type;
  }

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let make_substitution class_type class_parameters =
  Inst.make_subst class_type.dc_tparams class_parameters

let mark_as_synthesized inh =
  let mark_elt elt = Tuple.T2.map_fst elt ~f:set_elt_synthesized in
  {
    ih_substs =
      SMap.map
        begin
          (fun sc -> { sc with sc_from_req_extends = true })
        end
        inh.ih_substs;
    ih_cstr = (Option.map (fst inh.ih_cstr) ~f:mark_elt, snd inh.ih_cstr);
    ih_props = SMap.map mark_elt inh.ih_props;
    ih_sprops = SMap.map mark_elt inh.ih_sprops;
    ih_methods = SMap.map mark_elt inh.ih_methods;
    ih_smethods = SMap.map mark_elt inh.ih_smethods;
    ih_typeconsts =
      SMap.map
        (fun const -> { const with ttc_synthesized = true })
        inh.ih_typeconsts;
    ih_consts =
      SMap.map (fun const -> { const with cc_synthesized = true }) inh.ih_consts;
    ih_support_dynamic_type = inh.ih_support_dynamic_type;
  }

(*****************************************************************************)
(* Code filtering the private members (useful for inheritance) *)
(*****************************************************************************)

let is_private
    (type member)
    (module Member : Member_S with type t = member)
    (member : member) : bool =
  match Member.visibility member with
  | Vprivate _ -> not @@ Member.has_lsb member
  | Vpublic
  | Vprotected _
  | Vinternal _ ->
    false

let filter_privates class_type =
  let is_not_private _ elt = not @@ is_private (module Decl_defs_element) elt in
  {
    class_type with
    dc_props = SMap.filter is_not_private class_type.dc_props;
    dc_sprops = SMap.filter is_not_private class_type.dc_sprops;
    dc_methods = SMap.filter is_not_private class_type.dc_methods;
    dc_smethods = SMap.filter is_not_private class_type.dc_smethods;
  }

let chown_private_and_protected owner class_type =
  let chown elt =
    match elt.elt_visibility with
    | Vprivate _ -> { elt with elt_visibility = Vprivate owner }
    (* Update protected member that's included via a `use` statement, unless
     * it's synthesized, in which case its owner will be the one specified
     * in `requires extends` or `requires implements`
     *)
    | Vprotected _ when not (get_elt_synthesized elt) ->
      { elt with elt_visibility = Vprotected owner }
    | Vpublic
    | Vprotected _
    | Vinternal _ ->
      elt
  in
  {
    class_type with
    dc_props = SMap.map chown class_type.dc_props;
    dc_sprops = SMap.map chown class_type.dc_sprops;
    dc_methods = SMap.map chown class_type.dc_methods;
    dc_smethods = SMap.map chown class_type.dc_smethods;
  }

(*****************************************************************************)
(* Builds the inherited type when the class lives in Hack *)
(*****************************************************************************)

let pair_with_heap_entries :
    type heap_entry.
    element SMap.t ->
    heap_entry SMap.t option ->
    (element * heap_entry option) SMap.t =
 fun elts heap_entries ->
  SMap.mapi (fun name elt -> (elt, heap_entries >>= SMap.find_opt name)) elts

let inherit_hack_class
    child
    parent_name
    parent
    argl
    (parent_members : Decl_store.class_members option) : inherited =
  let subst = make_substitution parent argl in
  let parent =
    match parent.dc_kind with
    | Ast_defs.Ctrait ->
      (* Change the private/protected visibility to point to the inheriting class *)
      chown_private_and_protected (snd child.sc_name) parent
    | Ast_defs.Cclass _
    | Ast_defs.Cinterface ->
      filter_privates parent
    | Ast_defs.Cenum
    | Ast_defs.Cenum_class _ ->
      parent
  in
  let typeconsts =
    SMap.map (Inst.instantiate_typeconst_type subst) parent.dc_typeconsts
  in
  let consts = SMap.map (Inst.instantiate_cc subst) parent.dc_consts in
  let (cstr, constructor_consistency) = parent.dc_construct in
  let subst_ctx =
    {
      sc_subst = subst;
      sc_class_context = snd child.sc_name;
      sc_from_req_extends = false;
    }
  in
  let substs = SMap.add parent_name subst_ctx parent.dc_substs in
  let result =
    {
      ih_substs = substs;
      ih_cstr =
        ( Option.map cstr ~f:(fun cstr ->
              let constructor_heap_value =
                parent_members >>= fun m -> m.Decl_store.m_constructor
              in
              (cstr, constructor_heap_value)),
          constructor_consistency );
      ih_consts = consts;
      ih_typeconsts = typeconsts;
      ih_props =
        pair_with_heap_entries
          parent.dc_props
          (parent_members >>| fun m -> m.Decl_store.m_properties);
      ih_sprops =
        pair_with_heap_entries
          parent.dc_sprops
          (parent_members >>| fun m -> m.Decl_store.m_static_properties);
      ih_methods =
        pair_with_heap_entries
          parent.dc_methods
          (parent_members >>| fun m -> m.Decl_store.m_methods);
      ih_smethods =
        pair_with_heap_entries
          parent.dc_smethods
          (parent_members >>| fun m -> m.Decl_store.m_static_methods);
      ih_support_dynamic_type = parent.dc_support_dynamic_type;
    }
  in
  result

(* mostly copy paste of inherit_hack_class *)
let inherit_hack_class_constants_only class_type argl _parent_members =
  let subst = make_substitution class_type argl in
  let instantiate = SMap.map (Inst.instantiate_cc subst) in
  let consts = instantiate class_type.dc_consts in
  let typeconsts =
    SMap.map (Inst.instantiate_typeconst_type subst) class_type.dc_typeconsts
  in
  let result = { empty with ih_consts = consts; ih_typeconsts = typeconsts } in
  result

(* This logic deals with importing XHP attributes from an XHP class
   via the "attribute :foo;" syntax. *)
let inherit_hack_xhp_attrs_only class_type members =
  (* Filter out properties that are not XHP attributes *)
  let props =
    SMap.fold
      begin
        fun name prop acc ->
          if Option.is_some (get_elt_xhp_attr prop) then
            SMap.add name prop acc
          else
            acc
      end
      class_type.dc_props
      SMap.empty
  in
  let result =
    {
      empty with
      ih_props =
        pair_with_heap_entries
          props
          (members >>| fun m -> m.Decl_store.m_properties);
    }
  in
  result

(*****************************************************************************)

(* Include definitions inherited from a class (extends) or a trait (use)
 * or requires extends
 *)
let from_class c (parents : Decl_store.class_entries SMap.t) parent_ty :
    inherited =
  let (_, (_, parent_name), parent_class_params) =
    Decl_utils.unwrap_class_type parent_ty
  in
  match SMap.find_opt parent_name parents with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some (class_, parent_members) ->
    (* The class lives in Hack *)
    inherit_hack_class c parent_name class_ parent_class_params parent_members

let from_class_constants_only (parents : Decl_store.class_entries SMap.t) ty =
  let (_, (_, class_name), class_params) = Decl_utils.unwrap_class_type ty in
  match SMap.find_opt class_name parents with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some (class_, parent_members) ->
    (* The class lives in Hack *)
    inherit_hack_class_constants_only class_ class_params parent_members

let from_class_xhp_attrs_only (parents : Decl_store.class_entries SMap.t) ty =
  let (_, (_pos, class_name), _class_params) =
    Decl_utils.unwrap_class_type ty
  in
  match SMap.find_opt class_name parents with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some (class_, parent_members) ->
    (* The class lives in Hack *)
    inherit_hack_xhp_attrs_only class_ parent_members

let parents_which_provide_members c =
  (* /!\ For soundness, the traversal order below
   * must be consistent with traversal order for ancestors
   * used in decl_folded_class.ml *)
  (* In an abstract class or a trait, we assume the interfaces
   * will be implemented in the future, so we take them as
   * part of the class (as requested by dependency injection implementers) *)
  match c.sc_kind with
  | Ast_defs.Cclass k when Ast_defs.is_abstract k ->
    c.sc_implements @ c.sc_extends
  | Ast_defs.Ctrait -> c.sc_implements @ c.sc_extends @ c.sc_req_implements
  | Ast_defs.(Cclass _ | Cinterface | Cenum | Cenum_class _) -> c.sc_extends

let from_parent env c (parents : Decl_store.class_entries SMap.t) acc parent =
  let inherited = from_class c parents parent in
  add_inherited env c inherited acc

let from_requirements env c parents acc reqs =
  let inherited = from_class c parents reqs in
  let inherited = mark_as_synthesized inherited in
  add_inherited env c inherited acc

let from_trait env c parents acc uses =
  let inherited = from_class c parents uses in
  add_inherited env c inherited acc

let from_xhp_attr_use env c (parents : Decl_store.class_entries SMap.t) acc uses
    =
  let inherited = from_class_xhp_attrs_only parents uses in
  add_inherited env c inherited acc

(** Inherits constants and type constants from an interface. *)
let from_interface_constants
    env c (parents : Decl_store.class_entries SMap.t) acc impls =
  let inherited = from_class_constants_only parents impls in
  add_inherited env c inherited acc

let has_highest_precedence : (OverridePrecedence.t * 'a) option -> bool =
  function
  | Some (precedence, _) -> OverridePrecedence.is_highest precedence
  | _ -> false

let max_precedence (type a) :
    (OverridePrecedence.t * a) option ->
    (OverridePrecedence.t * a) option ->
    (OverridePrecedence.t * a) option =
  Option.merge ~f:(fun x y ->
      let (x_precedence, _) = x and (y_precedence, _) = y in
      if OverridePrecedence.(y_precedence > x_precedence) then
        y
      else
        x)

let ( >?? )
    (x : (OverridePrecedence.t * 'a) option)
    (y : (OverridePrecedence.t * 'a) option lazy_t) :
    (OverridePrecedence.t * 'a) option =
  if has_highest_precedence x then
    x
  else
    max_precedence x (Lazy.force y)

let find_first_with_highest_precedence
    (l : 'a list) ~(f : 'a -> (OverridePrecedence.t * _) option) :
    (OverridePrecedence.t * _) option =
  let rec loop found = function
    | [] -> found
    | x :: l ->
      let x = f x in
      if has_highest_precedence x then
        x
      else
        loop (max_precedence found x) l
  in
  loop None l

type parent_kind =
  | Parent
  | Requirement
  | Trait

type parent = decl_ty

module OrderedParents : sig
  type t

  (** This provides the parent traversal order for member folding.
    This is appropriate for all members except XHP attributes and constants.
    The order is different for those and these are handled elsewhere. *)
  val get : shallow_class -> t

  val fold : t -> init:'acc -> f:(parent_kind -> 'acc -> parent -> 'acc) -> 'acc

  (** Reverse the ordered parents. *)
  val rev : t -> t

  val find_map_first_with_highest_precedence :
    t ->
    f:(parent_kind -> parent -> (OverridePrecedence.t * 'res) option) ->
    (OverridePrecedence.t * 'res) option
end = struct
  type t = (parent_kind * parent list) list

  let get (c : shallow_class) : t =
    (* /!\ For soundness, the traversal order below
     * must be consistent with traversal order for ancestors
     * used in decl_folded_class.ml *)
    [
      (Parent, parents_which_provide_members c |> List.rev);
      (Requirement, c.sc_req_class @ c.sc_req_extends);
      (Trait, c.sc_uses);
    ]

  let fold (t : t) ~init ~(f : parent_kind -> 'acc -> parent -> 'acc) =
    List.fold t ~init ~f:(fun acc (parent_kind, parents) ->
        List.fold ~init:acc ~f:(f parent_kind) parents)

  let rev : t -> t = List.rev_map ~f:(Tuple2.map_snd ~f:List.rev)

  let find_map_first_with_highest_precedence
      (type res)
      (t : t)
      ~(f : parent_kind -> parent -> (OverridePrecedence.t * res) option) :
      (OverridePrecedence.t * res) option =
    List.fold t ~init:None ~f:(fun acc (parent_kind, parents) ->
        acc
        >?? lazy (find_first_with_highest_precedence parents ~f:(f parent_kind)))
end

let make env c ~cache:(parents : Decl_store.class_entries SMap.t) =
  let acc = empty in
  let acc =
    OrderedParents.get c
    |> OrderedParents.fold ~init:acc ~f:(function
           | Parent -> from_parent env c parents
           | Requirement -> from_requirements env c parents
           | Trait -> from_trait env c parents)
  in
  let acc =
    List.fold_left
      ~f:(from_xhp_attr_use env c parents)
      ~init:acc
      c.sc_xhp_attr_uses
  in
  (* todo: what about the same constant defined in different interfaces
   * we implement? We should forbid and say "constant already defined".
   * to julien: where is the logic that check for duplicated things?
   * todo: improve constant handling, see task #2487051
   *)
  let acc =
    List.fold_left
      ~f:(from_interface_constants env c parents)
      ~init:acc
      c.sc_req_implements
  in
  let included_enums =
    match c.sc_enum_type with
    | None -> []
    | Some enum_type -> enum_type.te_includes
  in
  let acc =
    List.fold_left
      ~f:(from_interface_constants env c parents)
      ~init:acc
      included_enums
  in
  List.fold_left
    ~f:(from_interface_constants env c parents)
    ~init:acc
    c.sc_implements

let find_overridden_method
    (cls : shallow_class) ~(get_method : decl_ty -> class_elt option) :
    class_elt option =
  let is_not_private m = not @@ is_private (module Typing_defs_class_elt) m in
  let precedence : class_elt -> OverridePrecedence.t =
    OverridePrecedence.make (module Typing_defs_class_elt)
  in
  let get_method_with_precedence parent_kind ty =
    match parent_kind with
    | Trait -> None
    | Parent
    | Requirement ->
      get_method ty |> Option.filter ~f:is_not_private >>| fun method_ ->
      (precedence method_, method_)
  in
  OrderedParents.get cls
  |> OrderedParents.rev
  |> OrderedParents.find_map_first_with_highest_precedence
       ~f:get_method_with_precedence
  >>| snd
