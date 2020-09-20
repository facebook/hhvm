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
open Reordered_argument_collections
open Shallow_decl_defs
open Typing_defs

(* Module calculating the Member Resolution Order of a class *)

(* NB: For incremental typechecking to be correct, every bit of information from
   shallow classes used in this module must be accounted for in the function
   Shallow_class_diff.mro_unchanged! *)

type env = {
  class_stack: SSet.t;
  decl_env: Decl_env.env;
  linearization_kind: linearization_kind;
}

let get_ctx env = env.decl_env.Decl_env.ctx

(** These state variants drive the Sequence generating the linearization. *)
type state =
  | Child of mro_element
      (** [Child] contains the first element in the linearization, the class which
      was linearized. *)
  | Next_ancestor
      (** [Next_ancestor] indicates that the next ancestor linearization should be
      lazily computed and emitted. *)
  | Ancestor of (string * linearization)
      (** [Ancestor] indicates that we are in the middle of emitting an ancestor
      linearization. For each of its elements, the element should be emitted as
      an element of the current linearization (with the appropriate source and
      type parameters substituted) unless it was already emitted earlier in the
      current linearization sequence. *)
  | Synthesized_elts of mro_element list
      (** A list of synthesized ancestor MRO elements (that is, classes which were
      specified in a require-extends clause, plus all ancestors of those
      classes) which were accumulated while we iterated over ancestor
      linearizations. These are called "synthesized" ancestors because although
      traits and interfaces don't actually inherit from them at runtime, the
      typechecker acts as if they do, in order to support typechecking of trait
      method bodies and uses of values with interface types. We want to
      de-prioritize require-extends ancestors by placing them at the end of the
      linearization, so that non-synthesized members (if they are present) are
      inherited instead of synthesized ones (since we are interested in the
      actual type arguments rather than the type arguments applied in the
      requirement). For each MRO element in the list, that element should be
      emitted as an element of the current linearization (with the appropriate
      type parameter substitutions) unless it was already emitted earlier in the
      current linearization sequence. *)

let ancestor_from_ty (source : source_type) (ty : decl_ty) :
    Pos.t * (Pos.t * string) * decl_ty list * source_type =
  let (r, class_name, type_args) = Decl_utils.unwrap_class_type ty in
  let ty_pos = Typing_reason.to_pos r in
  (ty_pos, class_name, type_args, source)

let from_parent (c : shallow_class) : decl_ty list =
  (* In an abstract class or a trait, we assume the interfaces
   * will be implemented in the future, so we take them as
   * part of the class (as requested by dependency injection implementers)
   *)
  match c.sc_kind with
  | Ast_defs.Cabstract -> c.sc_implements @ c.sc_extends
  | Ast_defs.Ctrait -> c.sc_implements @ c.sc_extends @ c.sc_req_implements
  | _ -> c.sc_extends

(* Return true if the two given MRO elements are equal in everything except name
   (which is expected to be compared by the caller) and positions (which are
   ignored). *)
let emitted_mro_elements_equal a b =
  let ( = ) = Bool.equal in
  phys_equal a b
  ||
  let {
    (* Ignore name and positions *)
    mro_name = _;
    mro_use_pos = _;
    mro_ty_pos = _;
    (* Compare everything else *)
    mro_flags;
    mro_cyclic;
    mro_required_at;
    mro_trait_reuse;
    mro_type_args;
  } =
    a
  in
  Int.equal mro_flags b.mro_flags
  && Option.is_none mro_cyclic = Option.is_none b.mro_cyclic
  && Option.is_none mro_required_at = Option.is_none b.mro_required_at
  && Option.equal String.equal mro_trait_reuse b.mro_trait_reuse
  && List.equal mro_type_args b.mro_type_args ~equal:equal_decl_ty

let empty_mro_element =
  {
    mro_name = "";
    mro_use_pos = Pos.none;
    mro_ty_pos = Pos.none;
    mro_flags = 0;
    mro_type_args = [];
    mro_cyclic = None;
    mro_trait_reuse = None;
    mro_required_at = None;
  }

let disallow_trait_reuse env =
  TypecheckerOptions.disallow_trait_reuse (Decl_env.tcopt env.decl_env)

let is_requirement (source : source_type) =
  match source with
  | ReqImpl
  | ReqExtends ->
    true
  | Child
  | Parent
  | Trait
  | XHPAttr
  | Interface
  | IncludedEnum ->
    false

let is_interface (source : source_type) =
  match source with
  | Interface
  | ReqImpl ->
    true
  | Child
  | Parent
  | Trait
  | XHPAttr
  | ReqExtends
  | IncludedEnum ->
    false

let is_includedEnum (source : source_type) =
  match source with
  | IncludedEnum -> true
  | Interface
  | ReqImpl
  | Child
  | Parent
  | Trait
  | XHPAttr
  | ReqExtends ->
    false

let rec ancestor_linearization
    (env : env)
    (child_class_concrete : bool)
    (ancestor : Pos.t * (Pos.t * string) * decl_ty list * source_type) :
    string * linearization =
  let (ty_pos, (use_pos, class_name), type_args, source) = ancestor in
  Decl_env.add_extends_dependency env.decl_env class_name;
  let lin = get_linearization env class_name in
  let lin =
    Sequence.map lin ~f:(fun c ->
        let via_req_extends =
          is_set mro_via_req_extends c.mro_flags
          || equal_source_type source ReqExtends
        in
        let via_req_impl =
          is_set mro_via_req_impl c.mro_flags
          || equal_source_type source ReqImpl
        in
        let xhp_attrs_only =
          is_set mro_xhp_attrs_only c.mro_flags
          || equal_source_type source XHPAttr
        in
        let consts_only =
          is_set mro_consts_only c.mro_flags
          || is_interface source
          || is_includedEnum source
        in
        let copy_private_members =
          is_set mro_copy_private_members c.mro_flags
          && equal_source_type source Trait
        in
        let passthrough_abstract_typeconst =
          is_set mro_passthrough_abstract_typeconst c.mro_flags
          && not child_class_concrete
        in
        let mro_flags =
          c.mro_flags
          |> set_bit mro_via_req_extends via_req_extends
          |> set_bit mro_via_req_impl via_req_impl
          |> set_bit mro_xhp_attrs_only xhp_attrs_only
          |> set_bit mro_consts_only consts_only
          |> set_bit mro_copy_private_members copy_private_members
          |> set_bit
               mro_passthrough_abstract_typeconst
               passthrough_abstract_typeconst
        in
        {
          c with
          mro_trait_reuse =
            Option.map c.mro_trait_reuse ~f:(Fn.const class_name);
          mro_flags;
        })
  in
  match Sequence.next lin with
  | None -> (class_name, Sequence.empty)
  | Some (c, rest) ->
    let c =
      {
        c with
        (* Fill in the type arguments applied to the ancestor and the position
         where it was included into the linearization of the child class. *)
        mro_type_args = type_args;
        mro_use_pos = use_pos;
        mro_ty_pos = ty_pos;
        (* If this is the mro_element representing an ancestor which directly
         appeared in a require extends clause, tag it as a requirement. *)
        mro_required_at =
          ( if is_requirement source then
            Some use_pos
          else
            None );
      }
    in
    let tparams =
      Shallow_classes_provider.get (get_ctx env) class_name
      |> Option.value_map ~default:[] ~f:(fun c -> c.sc_tparams)
    in
    let subst = Decl_subst.make_decl tparams type_args in
    let rest =
      Sequence.map rest ~f:(fun c ->
          {
            c with
            (* Instantiate the remainder of this ancestor's linearization with the
           type arguments applied to it. *)
            mro_type_args =
              List.map c.mro_type_args ~f:(Decl_instantiate.instantiate subst);
            (* Update positions of requirements in the remainder of the
           linearization to reflect their immediate provenance as well. *)
            mro_required_at = Option.map c.mro_required_at ~f:(fun _ -> use_pos);
          })
    in
    let rest =
      (* Don't aggregate requirements applied to ancestors. We have already
         verified that the ancestor extends and implements the required classes
         and interfaces, so there is no need to do so for the child. *)
      let ancestor_checks_requirements =
        Shallow_classes_provider.get (get_ctx env) class_name
        |> Option.value_map ~default:false ~f:(fun c ->
               match c.sc_kind with
               | Ast_defs.(Cnormal | Cabstract) -> true
               | Ast_defs.(Ctrait | Cinterface | Cenum) -> false)
      in
      if not ancestor_checks_requirements then
        rest
      else
        Sequence.map rest ~f:(fun mro -> { mro with mro_required_at = None })
    in
    (class_name, Sequence.append (Sequence.singleton c) rest)

(* Linearize a class declaration given its shallow declaration *)
and linearize (env : env) (c : shallow_class) : linearization =
  let mro_name = snd c.sc_name in
  (* The first class doesn't have its type parameters filled in *)
  let mro_flags =
    set_bit
      mro_copy_private_members
      (Ast_defs.equal_class_kind c.sc_kind Ast_defs.Ctrait)
      empty_mro_element.mro_flags
  in
  let mro_flags =
    set_bit
      mro_passthrough_abstract_typeconst
      (not Ast_defs.(equal_class_kind c.sc_kind Cnormal))
      mro_flags
  in
  let child =
    {
      empty_mro_element with
      mro_name;
      mro_use_pos = fst c.sc_name;
      mro_ty_pos = fst c.sc_name;
      mro_flags;
    }
  in
  let get_ancestors kind = List.map ~f:(ancestor_from_ty kind) in
  let interfaces c = get_ancestors Interface c.sc_implements in
  let req_implements c = get_ancestors ReqImpl c.sc_req_implements in
  let xhp_attr_uses c = get_ancestors XHPAttr c.sc_xhp_attr_uses in
  let traits c = get_ancestors Trait c.sc_uses in
  let req_extends c = get_ancestors ReqExtends c.sc_req_extends in
  let parents c = get_ancestors Parent (from_parent c) in
  let extends c = get_ancestors Parent c.sc_extends in
  let includes c =
    Aast.enum_includes_map c.sc_enum_type ~f:(fun enum_type ->
        get_ancestors IncludedEnum enum_type.te_includes)
  in
  (* HHVM implicitly adds the Stringish interface to every class, interface, and
     trait with a __toString method. The primitive type `string` is considered
     to also implement this interface. *)
  let stringish_interface c =
    let module SN = Naming_special_names in
    if String.equal mro_name SN.Classes.cStringish then
      []
    else
      let is_to_string m = String.equal (snd m.sm_name) SN.Members.__toString in
      match List.find c.sc_methods is_to_string with
      | None -> []
      | Some { sm_name = (pos, _); _ } ->
        let ty =
          mk (Typing_reason.Rhint pos, Tapply ((pos, SN.Classes.cStringish), []))
        in
        [ancestor_from_ty Interface ty]
  in
  let ancestors =
    match env.linearization_kind with
    | Member_resolution ->
      List.concat
        [
          List.rev (interfaces c);
          List.rev (includes c);
          List.rev (req_implements c);
          List.rev (xhp_attr_uses c);
          List.rev (traits c);
          List.rev (req_extends c);
          parents c;
        ]
    | Ancestor_types ->
      (* In order to match the historical handling of ancestor types (that is,
         the collection of "canonical" type parameterizations of each ancestor
         of a class, which is used in subtyping), we need to build the
         linearization in the order [extends; implements; uses]. Require-extends
         and require-implements relationships need to be included only to
         support Stringish (and can be removed here if we remove support for the
         magic Stringish type, or require it to be explicitly implemented). *)
      List.concat
        [
          extends c;
          req_extends c;
          req_implements c;
          stringish_interface c;
          interfaces c;
          traits c;
        ]
  in
  Sequence.unfold_step
    ~init:(Child child, ancestors, [], [])
    ~f:(next_state env mro_name c.sc_kind (Caml.Hashtbl.create 32))
  |> Sequence.memoize

and next_state
    (env : env)
    (class_name : string)
    (child_class_kind : Ast_defs.class_kind)
    (emitted_elements : (string, mro_element list) Caml.Hashtbl.t)
    (state, ancestors, acc, synths) =
  Sequence.Step.(
    let child_class_concrete =
      Ast_defs.equal_class_kind child_class_kind Ast_defs.Cnormal
    in
    let add_emitted mro =
      let list =
        match Caml.Hashtbl.find_opt emitted_elements mro.mro_name with
        | Some list -> mro :: list
        | None -> [mro]
      in
      Caml.Hashtbl.replace emitted_elements mro.mro_name list
    in
    let yield mro_element next_state ancestors synths =
      add_emitted mro_element;
      Yield (mro_element, (next_state, ancestors, mro_element :: acc, synths))
    in
    let was_emitted mro =
      match Caml.Hashtbl.find_opt emitted_elements mro.mro_name with
      | None -> false
      | Some list -> List.mem list mro ~equal:emitted_mro_elements_equal
    in
    let name_was_emitted mro = Caml.Hashtbl.mem emitted_elements mro.mro_name in
    match (state, ancestors) with
    | (Child child, _) -> yield child Next_ancestor ancestors synths
    | (Next_ancestor, ancestor :: ancestors) ->
      let name_and_lin =
        ancestor_linearization env child_class_concrete ancestor
      in
      Skip (Ancestor name_and_lin, ancestors, acc, synths)
    | (Ancestor (name, lin), ancestors) ->
      begin
        match Sequence.next lin with
        | None -> Skip (Next_ancestor, ancestors, acc, synths)
        (* Lazy.Undefined occurs if we attempt to include a linearization within
       itself. This will only happen when we have a class dependency cycle (and
       only in some particular circumstances), so it will not arise in legal
       programs. *)
        | exception Lazy.Undefined ->
          let next =
            {
              empty_mro_element with
              mro_name = name;
              mro_cyclic = Some (SSet.add env.class_stack name);
            }
          in
          yield next Next_ancestor ancestors synths
        | Some (next, rest) ->
          let skip_or_mark_trait_reuse was_emitted =
            let is_trait class_name =
              match Shallow_classes_provider.get (get_ctx env) class_name with
              | Some { sc_kind = Ast_defs.Ctrait; _ } -> true
              | _ -> false
            in
            if
              disallow_trait_reuse env
              && Option.is_none next.mro_trait_reuse
              && is_trait next.mro_name
            then
              (* When the disallow_trait_reuse feature is enabled, we want to report
             an error for reused traits. Instead of skipping trait
             mro_elements when they are already present in the
             linearization, we emit an element with the trait_reuse flag
             set so that we can error later. *)
              if name_was_emitted next then
                Some { next with mro_trait_reuse = Some name }
              else
                Some next
            else if was_emitted next then
              None
            else
              Some next
          in
          let (next, synths) =
            match env.linearization_kind with
            | Member_resolution ->
              if is_set mro_via_req_extends next.mro_flags then
                let synths =
                  match (next.mro_required_at, child_class_kind) with
                  (* Always aggregate synthesized ancestors for traits and
                 interfaces (necessary for typechecking) *)
                  | (_, Ast_defs.(Ctrait | Cinterface))
                  (* Otherwise, keep them only if they represent a requirement that
                 we will need to validate later. *)
                  | (Some _, Ast_defs.(Cnormal | Cabstract | Cenum)) ->
                    next :: synths
                  | (None, _) -> synths
                in
                (None, synths)
              else
                let next = skip_or_mark_trait_reuse was_emitted in
                (next, synths)
            | Ancestor_types ->
              (* For ancestor types, we don't care about require-extends or
             require-implements relationships, except for the fact that we want
             Stringish as an ancestor if we have some ancestor which requires
             it. *)
              let should_skip =
                ( is_set mro_via_req_extends next.mro_flags
                || is_set mro_via_req_impl next.mro_flags )
                && String.( <> ) next.mro_name SN.Classes.cStringish
              in
              let next =
                if should_skip then
                  None
                else
                  skip_or_mark_trait_reuse name_was_emitted
              in
              (next, synths)
          in
          (match next with
          | None -> Skip (Ancestor (name, rest), ancestors, acc, synths)
          | Some next -> yield next (Ancestor (name, rest)) ancestors synths)
      end
    | (Next_ancestor, []) ->
      let synths = List.rev synths in
      Skip (Synthesized_elts synths, ancestors, acc, synths)
    | (Synthesized_elts (next :: synths), ancestors) ->
      if was_emitted next then
        Skip (Synthesized_elts synths, ancestors, acc, synths)
      else
        yield next (Synthesized_elts synths) ancestors synths
    | (Synthesized_elts [], _) ->
      let key = (class_name, env.linearization_kind) in
      Linearization_provider.complete (get_ctx env) key (List.rev acc);
      Done)

and get_linearization (env : env) (class_name : string) : linearization =
  let { class_stack; linearization_kind; _ } = env in
  if SSet.mem class_stack class_name then
    Sequence.singleton
      {
        empty_mro_element with
        mro_name = class_name;
        mro_cyclic = Some class_stack;
      }
  else
    let class_stack = SSet.add class_stack class_name in
    let env = { env with class_stack } in
    let key = (class_name, linearization_kind) in
    match Linearization_provider.get (get_ctx env) key with
    | Some lin -> lin
    | None ->
      (match Shallow_classes_provider.get (get_ctx env) class_name with
      | Some c ->
        let lin = linearize env c in
        Linearization_provider.add (get_ctx env) key lin;
        lin
      | None ->
        (* There is no known definition for the class with the given name. This
          is always an "Unbound name" error, and we will emit one wherever this
          class was specified as an ancestor. In order to suppress downstream
          errors (and, historically, to support the now-removed assume_php
          feature), we include this fake mro_element with the
          mro_class_not_found flag set. This logic is largely here to ensure
          that the behavior of shallow_class_decl is equivalent to legacy decl,
          and we should look into removing it (along with
          Typing_classes_heap.members_fully_known) after we have removed legacy
          decl. *)
        let mro_flags =
          set_bit mro_class_not_found true empty_mro_element.mro_flags
        in
        (* This class is not known to exist! *)
        Sequence.singleton
          { empty_mro_element with mro_name = class_name; mro_flags })

let get_linearization
    (ctx : Provider_context.t) (key : string * Decl_defs.linearization_kind) :
    Decl_defs.linearization =
  let (class_name, kind) = key in
  let decl_env =
    {
      Decl_env.mode = FileInfo.Mstrict;
      droot = Some (Typing_deps.Dep.Class class_name);
      ctx;
    }
  in
  let env = { class_stack = SSet.empty; decl_env; linearization_kind = kind } in
  get_linearization env class_name
