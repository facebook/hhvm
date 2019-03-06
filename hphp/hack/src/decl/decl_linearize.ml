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

module Cache = SharedMem.LocalCache (StringKey) (struct
  type t = linearization
  let prefix = Prefix.make()
  let description = "LazyLinearization"
  let use_sqlite_fallback () = false
end)

let ancestor_from_ty
    (new_source : source_type)
    (ty : decl ty)
  : string * decl ty list * source_type =
  let _, (_, class_name), type_args = Decl_utils.unwrap_class_type ty in
  class_name, type_args, new_source

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
    (class_name : string)
    (type_args : decl ty list)
    (new_source : source_type)
  : linearization =
  Decl_env.add_extends_dependency env.decl_env class_name;
  let lin = get_linearization env class_name in
  let lin = Sequence.map lin ~f:begin fun c ->
    let is_synthesized = function
      | ReqImpl | ReqExtends -> true
      | Child | Parent | Trait | XHPAttr | Interface -> false
    in
    let mro_synthesized = c.mro_synthesized || is_synthesized new_source in
    { c with
      mro_source = new_source;
      mro_synthesized;
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
    mro_source = Child;
    mro_synthesized = false;
  } in
  let ancestors = List.concat [
    (* Add traits in backwards order *)
    List.map (List.rev c.sc_uses) (ancestor_from_ty Trait);
    (* Add interfaces(interfaces can define constants)
    TODO(jjwu): implemented interfaces are *only* important for constants and
    otherwise don't need to take up so much space in the linearization.
    Can we get rid of this somehow? *)
    List.map c.sc_implements (ancestor_from_ty Interface);
    (* Same with req_implements *)
    List.map c.sc_req_implements (ancestor_from_ty ReqImpl);
    (* Add requirements *)
    List.map c.sc_req_extends (ancestor_from_ty ReqExtends);
    List.map c.sc_xhp_attr_uses (ancestor_from_ty XHPAttr);
    List.map (from_parent c) (ancestor_from_ty Parent);
  ] in
  Sequence.unfold_step ~init:(Child child, ancestors, []) ~f:(next_state env)
  |> Sequence.memoize

and next_state (env : env) (state, ancestors, acc) =
  let open Sequence.Step in
  match state, ancestors with
  | Child child, _ -> Yield (child, (Next_ancestor, ancestors, child::acc))
  | Next_ancestor, [] -> Done
  | Next_ancestor, (cid, params, src)::ancestors ->
    Skip (Ancestor (ancestor_linearization env cid params src), ancestors, acc)
  | Ancestor lin, ancestors ->
    match Sequence.next lin with
    | None -> Skip (Next_ancestor, ancestors, acc)
    | Some (next, rest) ->
      if List.mem acc next ~equal:(=)
      then Skip (Ancestor rest, ancestors, acc)
      else Yield (next, (Ancestor rest, ancestors, next::acc))

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
