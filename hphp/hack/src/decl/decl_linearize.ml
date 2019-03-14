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
open Reordered_argument_collections
open Shallow_decl_defs
open Typing_defs

(* Module calculating the Member Resolution Order of a class *)

type env = { class_stack: SSet.t; decl_env: Decl_env.env }

(** These state variants drive the Sequence generating the linearization. *)
type state =
  | Child of mro_element
  (** [Child] contains the first element in the linearization, the class which
      was linearized. *)
  | Next_ancestor
  (** [Next_ancestor] indicates that the next ancestor linearization should be
      lazily computed and emitted. *)
  | Ancestor of linearization
  (** [Ancestor] indicates that we are in the middle of emitting an ancestor
      linearization. For each of its elements, the element should be emitted as
      an element of the current linearization (with the appropriate source and
      type parameters substituted) unless it was already emitted earlier in the
      current linearization sequence. *)
  | Synthesized_elts of mro_element list
  (** A list of synthesized ancestor MRO elements which were accumulated while
      we iterated over ancestor linearizations. We want to de-prioritize
      synthesized ancestors (i.e., ancestors arising from a require-extends or
      require-implements relationship) by placing them at the end of the
      linearization, so that non-synthesized members (if they are present) are
      inherited instead of synthesized ones. For each MRO element in the list,
      that element should be emitted as an element of the current linearization
      (with the appropriate source and type parameters substituted) unless it
      was already emitted earlier in the current linearization sequence. *)

module Cache = SharedMem.LocalCache (StringKey) (struct
  type t = linearization
  let prefix = Prefix.make()
  let description = "LazyLinearization"
  let use_sqlite_fallback () = false
end)

let ancestor_from_ty
    (source : source_type)
    (ty : decl ty)
  : string * decl ty list * source_type =
  let _, (_, class_name), type_args = Decl_utils.unwrap_class_type ty in
  class_name, type_args, source

let from_parent (c : shallow_class) : decl ty list =
  (* In an abstract class or a trait, we assume the interfaces
   * will be implemented in the future, so we take them as
   * part of the class (as requested by dependency injection implementers)
   *)
  match c.sc_kind with
  | Ast.Cabstract -> c.sc_implements @ c.sc_extends
  | Ast.Ctrait -> c.sc_implements @ c.sc_extends @ c.sc_req_implements
  | _ -> c.sc_extends

let rec ancestor_linearization
    (env : env)
    (ancestor : string * decl ty list * source_type)
  : linearization =
  let class_name, type_args, source = ancestor in
  Decl_env.add_extends_dependency env.decl_env class_name;
  let lin = get_linearization env class_name in
  let lin = Sequence.map lin ~f:begin fun c ->
    let is_synthesized = function
      | ReqImpl | ReqExtends -> true
      | Child | Parent | Trait | XHPAttr | Interface -> false
    in
    let is_interface = function
      | Interface | ReqImpl -> true
      | Child | Parent | Trait | XHPAttr | ReqExtends -> false
    in
    let mro_synthesized = c.mro_synthesized || is_synthesized source in
    let mro_xhp_attrs_only = c.mro_xhp_attrs_only || source = XHPAttr in
    let mro_consts_only = c.mro_consts_only || is_interface source in
    let mro_copy_private_members = c.mro_copy_private_members && source = Trait in
    { c with
      mro_synthesized;
      mro_xhp_attrs_only;
      mro_consts_only;
      mro_copy_private_members;
    }
  end in
  match Sequence.next lin with
  | None -> Sequence.empty
  | Some (c, rest) ->
    (* Fill in the type parameterization of the starting class *)
    let c = { c with mro_type_args = type_args } in
    (* Instantiate its linearization with those type parameters *)
    let tparams =
      Shallow_classes_heap.get class_name
      |> Option.value_map ~default:[] ~f:(fun c -> c.sc_tparams)
    in
    let subst = Decl_subst.make tparams type_args in
    let rest = Sequence.map rest ~f:begin fun c ->
      { c with mro_type_args =
        List.map c.mro_type_args ~f:(Decl_instantiate.instantiate subst)
      }
    end in
    Sequence.append (Sequence.singleton c) rest

(* Linearize a class declaration given its shallow declaration *)
and linearize (env : env) (c : shallow_class) : linearization =
  let mro_name = snd c.sc_name in
  (* The first class doesn't have its type parameters filled in *)
  let child = {
    mro_name;
    mro_type_args = [];
    mro_synthesized = false;
    mro_xhp_attrs_only = false;
    mro_consts_only = false;
    mro_copy_private_members = c.sc_kind = Ast.Ctrait;
  } in
  let get_ancestors kind = List.map ~f:(ancestor_from_ty kind) in
  let interfaces     = get_ancestors Interface c.sc_implements in
  let req_implements = get_ancestors ReqImpl c.sc_req_implements in
  let xhp_attr_uses  = get_ancestors XHPAttr (List.rev c.sc_xhp_attr_uses) in
  let traits         = get_ancestors Trait (List.rev c.sc_uses) in
  let req_extends    = get_ancestors ReqExtends c.sc_req_extends in
  let parents        = get_ancestors Parent (from_parent c) in
  let ancestors =
    List.concat [
      interfaces;
      req_implements;
      xhp_attr_uses;
      traits;
      req_extends;
      parents;
    ]
  in
  Sequence.unfold_step
    ~init:(Child child, ancestors, [], [])
    ~f:(next_state env)
  |> Sequence.memoize

and next_state (env : env) (state, ancestors, acc, synths) =
  let open Sequence.Step in
  match state, ancestors with
  | Child child, _ -> Yield (child, (Next_ancestor, ancestors, child::acc, synths))
  | Next_ancestor, ancestor::ancestors ->
    Skip (Ancestor (ancestor_linearization env ancestor), ancestors, acc, synths)
  | Ancestor lin, ancestors ->
    begin match Sequence.next lin with
    | None -> Skip (Next_ancestor, ancestors, acc, synths)
    (* Lazy.Undefined occurs if we attempt to include a linearization within
       itself. This will only happen when we have a class dependency cycle (and
       only in some particular circumstances), so it will not arise in legal
       programs. *)
    | exception Lazy.Undefined -> Skip (Next_ancestor, ancestors, acc, synths)
    | Some (next, rest) ->
      let should_skip, synths =
        if next.mro_synthesized
        then true, next::synths
        else List.mem acc next ~equal:(=), synths
      in
      if should_skip
      then Skip (Ancestor rest, ancestors, acc, synths)
      else Yield (next, (Ancestor rest, ancestors, next::acc, synths))
    end
  | Next_ancestor, [] ->
    let synths = List.rev synths in
    Skip (Synthesized_elts synths, ancestors, acc, synths)
  | Synthesized_elts (next::synths), ancestors ->
    if List.mem acc next ~equal:(=)
    then Skip (Synthesized_elts synths, ancestors, next::acc, synths)
    else Yield (next, (Synthesized_elts synths, ancestors, next::acc, synths))
  | Synthesized_elts [], _ -> Done

and get_linearization (env : env) (class_name : string) : linearization =
  let { class_stack; _ } = env in
  if SSet.mem class_stack class_name then Sequence.empty else
  let class_stack = SSet.add class_stack class_name in
  let env = { env with class_stack } in
  match Cache.get class_name with
  | Some lin -> lin
  | None ->
    match Shallow_classes_heap.get class_name with
    | None -> Sequence.empty
    | Some c ->
      let lin = linearize env c in
      Cache.add class_name lin;
      lin

let get_linearization class_name =
  let decl_env = { Decl_env.
    mode = FileInfo.Mstrict;
    droot = Some (Typing_deps.Dep.Class class_name);
    decl_tcopt = GlobalNamingOptions.get ();
  } in
  get_linearization { class_stack = SSet.empty; decl_env } class_name
