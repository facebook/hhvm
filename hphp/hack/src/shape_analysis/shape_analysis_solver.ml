(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module T = Typing_defs
module Logic = Shape_analysis_logic
module HT = Hips_types

module StaticAccess = struct
  type ty = entity_ * T.TShapeField.t * Typing_defs.locl_ty

  let compare =
    Tuple.T3.compare
      ~cmp1:compare_entity_
      ~cmp2:T.TShapeField.compare
      ~cmp3:Typing_defs.compare_locl_ty

  module Set = struct
    module S = Caml.Set.Make (struct
      type t = ty

      let compare = compare
    end)

    include S
    include CommonSet (S)
  end
end

type constraints = {
  markers: (marker_kind * Pos.t) list;
  definitely_has_static_accesses: StaticAccess.Set.t;
  maybe_has_static_accesses: StaticAccess.Set.t;
  definitely_needs_static_accesses: StaticAccess.Set.t;
  maybe_needs_static_accesses: StaticAccess.Set.t;
  dynamic_accesses: EntitySet.t;
  subsets: (entity_ * entity_) list;
}

let constraints_init =
  {
    markers = [];
    definitely_has_static_accesses = StaticAccess.Set.empty;
    maybe_has_static_accesses = StaticAccess.Set.empty;
    definitely_needs_static_accesses = StaticAccess.Set.empty;
    maybe_needs_static_accesses = StaticAccess.Set.empty;
    dynamic_accesses = EntitySet.empty;
    subsets = [];
  }

let disassemble constraints =
  let partition_constraint constraints = function
    | Marks (kind, entity) ->
      { constraints with markers = (kind, entity) :: constraints.markers }
    | Static_key (Has, Definite, entity, key, ty) ->
      {
        constraints with
        definitely_has_static_accesses =
          StaticAccess.Set.add
            (entity, key, ty)
            constraints.definitely_has_static_accesses;
      }
    | Static_key (Has, Maybe, entity, key, ty) ->
      {
        constraints with
        maybe_has_static_accesses =
          StaticAccess.Set.add
            (entity, key, ty)
            constraints.maybe_has_static_accesses;
      }
    | Static_key (Needs, Definite, entity, key, ty) ->
      {
        constraints with
        definitely_needs_static_accesses =
          StaticAccess.Set.add
            (entity, key, ty)
            constraints.definitely_needs_static_accesses;
      }
    | Static_key (Needs, Maybe, entity, key, ty) ->
      {
        constraints with
        maybe_needs_static_accesses =
          StaticAccess.Set.add
            (entity, key, ty)
            constraints.maybe_needs_static_accesses;
      }
    | Has_dynamic_key entity ->
      {
        constraints with
        dynamic_accesses = EntitySet.add entity constraints.dynamic_accesses;
      }
    | Subsets (sub, sup) ->
      { constraints with subsets = (sub, sup) :: constraints.subsets }
  in
  List.fold ~init:constraints_init ~f:partition_constraint constraints

let assemble
    {
      markers;
      definitely_has_static_accesses;
      maybe_has_static_accesses;
      definitely_needs_static_accesses;
      maybe_needs_static_accesses;
      dynamic_accesses;
      subsets;
    } =
  List.map ~f:(fun (kind, entity) -> Marks (kind, entity)) markers
  @ List.map
      ~f:(fun (entity, key, ty) -> Static_key (Has, Definite, entity, key, ty))
      (StaticAccess.Set.elements definitely_has_static_accesses)
  @ List.map
      ~f:(fun (entity, key, ty) -> Static_key (Has, Maybe, entity, key, ty))
      (StaticAccess.Set.elements maybe_has_static_accesses)
  @ List.map
      ~f:(fun (entity, key, ty) ->
        Static_key (Needs, Definite, entity, key, ty))
      (StaticAccess.Set.elements definitely_needs_static_accesses)
  @ List.map
      ~f:(fun (entity, key, ty) -> Static_key (Needs, Maybe, entity, key, ty))
      (StaticAccess.Set.elements maybe_needs_static_accesses)
  @ List.map
      ~f:(fun entity -> Has_dynamic_key entity)
      (EntitySet.elements dynamic_accesses)
  @ List.map ~f:(fun (sub, sup) -> Subsets (sub, sup)) subsets

type adjacencies = {
  backwards: EntitySet.t;
  forwards: EntitySet.t;
}

let mk_adjacency_map subsets =
  let entities =
    List.concat_map ~f:(fun (e1, e2) -> [e1; e2]) subsets |> EntitySet.of_list
  in
  let backwards_of e (e1, e2) =
    if equal_entity_ e e2 then
      Some e1
    else
      None
  in
  let forwards_of e (e1, e2) =
    if equal_entity_ e e1 then
      Some e2
    else
      None
  in
  let find_adjacent of_ entity =
    List.filter_map ~f:(of_ entity) subsets |> EntitySet.of_list
  in
  let adjacency e =
    {
      backwards = find_adjacent backwards_of e;
      forwards = find_adjacent forwards_of e;
    }
  in
  let add e = EntityMap.add e (adjacency e) in
  EntitySet.fold add entities EntityMap.empty

(*
  Propagates `Static_key` constraints conjunctively through the dataflow graph.
  The propagation only happens if all incident edges have the relevant static
  key, e.g.,

    ('a', int) -------\               ('a', int) -------\
    ('b', string)      \              ('b', string)      \
                        \                                 \
    ('a', float) -------- .   ====>   ('a', float) -------- ('a', int)
                        /                                 / ('a', float)
                       /                                 /
    ('a', int) -------/               ('a', int) -------/

  We use universal quantification over incident edges which in general is not
  monotonic (hence not necessarily convergent in a fixpoint setting), however,
  the newly introduced edges in a growing adjacency map due to fixpoint
  computation cannot change the incident edges of any vertices. So, there is no
  way of invalidating a definite derived has static key constraint once it is
  established.

  TODO(T129093160): The monotonicity argument above is not strictly true due to
  parameters with default values.

  The implementation is semi-naïve, i.e., at each iteration it only considers
  what newly generated facts can affect.

    static_key(has, definite, E, K, Ty) :- static_key_base(has, definite, E, K, Ty)
    static_key(has, definite, E, K, Ty) :-
      common_definite_predecessor_key(E, K),
      subsets(E', E),
      static_key(has, definite, E', K, Ty).

    common_definite_predecessor_key(E, K) :-
      forall subsets(E', E).
      static_key(has, definite, E', K, _).
*)
let derive_definitely_has_static_accesses adjacency_map static_accesses =
  let open StaticAccess.Set in
  (* All successors to the entities in recently generated facts can potentially
     obtain a new static key constraint. *)
  let find_candidates delta =
    let accum (e, _, _) =
      let nexts =
        EntityMap.find_opt e adjacency_map
        |> Option.value_map ~default:EntitySet.empty ~f:(fun a -> a.forwards)
      in
      EntitySet.union nexts
    in
    fold accum delta EntitySet.empty
  in
  let rec close_upwards ~delta ~acc =
    if is_empty delta then
      acc
    else
      (* The following collects the intersection of all keys that occur in
         predecessors of an entity along with possible types of those keys. *)
      let common_predecessor_keys e =
        let predecessors =
          EntityMap.find_opt e adjacency_map
          |> Option.value_map ~default:EntitySet.empty ~f:(fun a -> a.backwards)
        in
        let collect_common_keys e common_keys =
          let collect_keys (e', k, ty) =
            let add_ty = function
              | None -> Some [ty]
              | Some tys -> Some (ty :: tys)
            in
            if equal_entity_ e e' then
              T.TShapeMap.update k add_ty
            else
              Fn.id
          in
          let keys = fold collect_keys acc T.TShapeMap.empty in
          (* Here `None` represents the universe set *)
          match common_keys with
          | None -> Some keys
          | Some keys' ->
            let combine _ tys_opt tys'_opt =
              match (tys_opt, tys'_opt) with
              | (Some tys, Some tys') -> Some (tys @ tys')
              | _ -> None
            in
            Some (T.TShapeMap.merge combine keys keys')
        in
        EntitySet.fold collect_common_keys predecessors None
        |> Option.value ~default:T.TShapeMap.empty
      in
      (* Propagate all common keys forward *)
      let propagate e acc =
        let common_predecessor_keys = common_predecessor_keys e in
        T.TShapeMap.fold
          (fun k tys ->
            union (List.map ~f:(fun ty -> (e, k, ty)) tys |> of_list))
          common_predecessor_keys
          acc
      in
      let candidates = find_candidates delta in
      let delta = EntitySet.fold propagate candidates StaticAccess.Set.empty in
      let delta = diff delta acc in
      let acc = union delta acc in
      close_upwards ~delta ~acc
  in
  close_upwards ~delta:static_accesses ~acc:static_accesses

(*
  Propagates `Static_key` constraints forward (or backward depending on
  variety) through the dataflow graph.

  The implementation is semi-naïve, i.e., at each iteration it only considers
  newly generated facts.

    static_key(has, maybe, E, Key, Ty), :- static_key_base(has, _, E, Key, Ty).
    static_key(has, maybe, F, Key, Ty) :- static_key(has, maybe, E, Key, Ty), subsets(E,F).

  The following is the dual of above with graph edges flipped during
  propagation. Here, unlike the forward case, we propagate both the
  `Definitive` and `Maybe` variants.

    static_key(needs, Certainty, E, Key, Ty) :- static_key_base(needs, Certainty, E, Key, Ty).
    static_key(needs, Certainty, F, Key, Ty) :- static_key(needs, Certainty, E, Key, Ty), subsets(F,E).
*)
let derive_disjunctive_static_accesses adjacency_map variety static_accesses =
  let open StaticAccess.Set in
  let rec close_upwards ~delta ~acc =
    if is_empty delta then
      acc
    else
      let propagate (e, k, ty) =
        EntityMap.find_opt e adjacency_map
        |> Option.value_map ~default:empty ~f:(fun adjacency ->
               let adjacency =
                 match variety with
                 | Has -> adjacency.forwards
                 | Needs -> adjacency.backwards
               in
               EntitySet.fold (fun e -> add (e, k, ty)) adjacency empty)
      in
      let delta = unions_map ~f:propagate delta in
      let delta = diff delta acc in
      let acc = union delta acc in
      close_upwards ~delta ~acc
  in
  close_upwards ~delta:static_accesses ~acc:static_accesses

(*
  Close dynamic key access in both directions to later invalidate all results
  that touch it.

    has_dynamic_key(E) :- has_dynamic_key_base(E).
    has_dynamic_key(F) :- has_dynamic_key_ud(E), (subsets(E,F); subsets(F,E)).
*)
let derive_dynamic_accesses adjacency_map dynamic_accesses =
  let open EntitySet in
  let rec close_upwards_and_downwards ~delta ~acc =
    if is_empty delta then
      acc
    else
      let propagate e =
        match EntityMap.find_opt e adjacency_map with
        | Some adjacency -> union adjacency.forwards adjacency.backwards
        | None -> empty
      in
      let delta = unions_map ~f:propagate delta in
      let delta = diff delta acc in
      let acc = union delta acc in
      close_upwards_and_downwards ~delta ~acc
  in
  close_upwards_and_downwards ~delta:dynamic_accesses ~acc:dynamic_accesses

(* The following program roughly summarises the solver in Datalog.

  Variables with single letters E and F and their variants with primes all
  range over entities.

  Comma (,) is used for conjunction semi-colon (;) is used for disjunction.
  Comma has higher precedence (binds tighter) than semi-colon.

  p :- q1, ..., qn

  means if q1 to qn holds, so does p.
*)
let deduce (constraints : constraint_ list) : constraint_ list =
  let {
    markers;
    definitely_has_static_accesses;
    maybe_has_static_accesses;
    definitely_needs_static_accesses;
    maybe_needs_static_accesses;
    dynamic_accesses;
    subsets;
  } =
    disassemble constraints
  in
  let adjacency_map = mk_adjacency_map subsets in

  (* Close upwards *)
  let maybe_has_static_accesses =
    derive_disjunctive_static_accesses
      adjacency_map
      Has
      (StaticAccess.Set.union
         maybe_has_static_accesses
         definitely_has_static_accesses)
  in

  (* Close upwards *)
  let definitely_has_static_accesses =
    derive_definitely_has_static_accesses
      adjacency_map
      definitely_has_static_accesses
  in

  (* Close downwards *)
  let definitely_needs_static_accesses =
    derive_disjunctive_static_accesses
      adjacency_map
      Needs
      definitely_needs_static_accesses
  in

  (* Close downwards *)
  let maybe_needs_static_accesses =
    derive_disjunctive_static_accesses
      adjacency_map
      Needs
      maybe_needs_static_accesses
  in

  (* Close upwards and downwards *)
  let dynamic_accesses =
    derive_dynamic_accesses adjacency_map dynamic_accesses
  in
  assemble
    {
      markers;
      definitely_has_static_accesses;
      maybe_has_static_accesses;
      definitely_needs_static_accesses;
      maybe_needs_static_accesses;
      dynamic_accesses;
      subsets;
    }

(*
  static_shape_result(E) :- marks(E), not has_dynamic_key_ud(E).
  static_shape_result_key(allocation | return | debug,E,Key,Ty) :-
    static_key(has,_,E,Key,Ty).
  static_shape_result_key(parameter,E,Key,Ty) :-
    static_key(needs,_,E,Key,Ty).
  static_shape_result_key_optional(allocation | return | debug,E,Key) :-
    has_static_key(has, _, E, Key, _),
    not has_static_key(has, definite, E, Key, _).
  static_shape_result_key_optional(parameter,E,Key) :-
    has_static_key(needs, maybe, E, Key, _).

  dynamic_shape_result(E) :- marks(E), has_dynamic_key(E).
*)
let produce_results
    (env : Typing_env_types.env) (constraints : constraint_ list) :
    shape_result list =
  let {
    markers;
    definitely_has_static_accesses;
    maybe_has_static_accesses;
    definitely_needs_static_accesses;
    maybe_needs_static_accesses;
    dynamic_accesses;
    _;
  } =
    disassemble constraints
  in

  (* Start collecting shape results starting with empty shapes of candidates *)
  let static_shape_results : (marker_kind * shape_keys) Pos.Map.t =
    markers
    |> List.fold
         ~f:(fun map (kind, pos) ->
           Pos.Map.add pos (kind, T.TShapeMap.empty) map)
         ~init:Pos.Map.empty
  in

  (* Invalidate candidates that are observed to experience dynamic access *)
  let dynamic_poss =
    let add entity acc =
      match entity with
      | Literal pos
      | Inter (HT.Param ((pos, _), _)) ->
        Pos.Set.add pos acc
      | Variable _
      | Inter (HT.Constant _)
      | Inter (HT.Identifier _) ->
        acc
    in
    EntitySet.fold add dynamic_accesses Pos.Set.empty
  in
  let static_shape_results : (marker_kind * shape_keys) Pos.Map.t =
    static_shape_results
    |> Pos.Map.filter (fun pos _ -> not @@ Pos.Set.mem pos dynamic_poss)
  in

  let (forward_static_shape_results, backward_static_shape_results) =
    Pos.Map.partition
      (fun _ (kind, _) ->
        match kind with
        | Parameter -> false
        | Debug
        | Return
        | Allocation
        | Constant ->
          true)
      static_shape_results
  in

  (* Add known keys *)
  let add_known_keys ~is_optional static_accesses static_shape_results :
      (marker_kind * shape_keys) Pos.Map.t =
    let update_entity _ key ty = function
      | None -> None
      | Some (kind, shape_keys') ->
        Some (kind, Logic.(singleton key ty is_optional <> shape_keys') ~env)
    in
    let add_entity (entity, key, ty) pos_map =
      match entity with
      | Literal pos
      | Inter (HT.Param ((pos, _), _))
      | Inter (HT.Constant (pos, _))
      | Inter (HT.Identifier (pos, _)) ->
        Pos.Map.update pos (update_entity entity key ty) pos_map
      | Variable _ -> pos_map
    in
    StaticAccess.Set.fold add_entity static_accesses static_shape_results
  in
  let forward_static_shape_results =
    let add_known_keys = add_known_keys in
    forward_static_shape_results
    |> add_known_keys maybe_has_static_accesses ~is_optional:true
    |> add_known_keys definitely_has_static_accesses ~is_optional:false
  in

  let backward_static_shape_results =
    let add_known_keys = add_known_keys in
    backward_static_shape_results
    |> add_known_keys maybe_needs_static_accesses ~is_optional:true
    |> add_known_keys definitely_needs_static_accesses ~is_optional:false
  in

  let static_shape_results =
    Pos.Map.union forward_static_shape_results backward_static_shape_results
  in

  (* Convert to individual statically accessed dict results *)
  let static_shape_results : shape_result list =
    static_shape_results
    |> Pos.Map.bindings
    |> List.map ~f:(fun (pos, (marker_kind, keys_and_types)) ->
           Shape_like_dict (pos, marker_kind, keys_and_types))
  in

  let dynamic_shape_results =
    dynamic_accesses
    |> EntitySet.filter (function
           | Variable _ -> false
           | _ -> true)
    |> EntitySet.elements
    |> List.map ~f:(fun entity_ -> Dynamically_accessed_dict entity_)
  in

  static_shape_results @ dynamic_shape_results

let embed_entity (ent : HT.entity) : entity_ = Inter ent

let substitute_inter_intra
    replace (inter_constr : inter_constraint_) (intra_constr : constraint_) :
    constraint_ option =
  let replace_intra intra_constr replace forwards =
    match intra_constr with
    | Marks _
    | Subsets (_, _) ->
      None
    | Static_key (variety, certainty, intra_ent_2, key, ty) ->
      let (variety, certainty) =
        if forwards then
          (Needs, Maybe)
        else
          (variety, certainty)
      in
      Option.map
        ~f:(fun x -> Static_key (variety, certainty, x, key, ty))
        (replace intra_ent_2)
    | Has_dynamic_key intra_ent_2 ->
      Option.map ~f:(fun x -> Has_dynamic_key x) (replace intra_ent_2)
  in
  match inter_constr with
  | HT.Arg (param_ent, intra_ent_1) ->
    let (replace_, forwards) =
      match replace with
      | `Backwards replace_ -> (replace_, false)
      | `Forwards replace_ -> (replace_, true)
    in
    let replace intra_ent_2 = replace_ param_ent intra_ent_1 intra_ent_2 in
    replace_intra intra_constr replace forwards
  | HT.ConstantInitial ent ->
    let replace intra_ent_2 =
      if equal_entity_ ent intra_ent_2 then
        Some ent
      else
        None
    in
    replace_intra intra_constr replace false
  | HT.Identifier ident_ent ->
    let ent = embed_entity (HT.Identifier ident_ent) in
    let replace intra_ent_2 =
      if equal_entity_ ent intra_ent_2 then
        Some ent
      else
        None
    in
    replace_intra intra_constr replace false
  | _ -> Some intra_constr

let replace_backwards
    (param_ent : HT.param_entity)
    (intra_ent_1 : entity_)
    (intra_ent_2 : entity_) : entity =
  if equal_entity_ (Inter (HT.Param param_ent)) intra_ent_2 then
    Some intra_ent_1
  else
    None

let replace_forwards
    (param_ent : HT.param_entity)
    (intra_ent_1 : entity_)
    (intra_ent_2 : entity_) : entity =
  if equal_entity_ intra_ent_1 intra_ent_2 then
    Some (embed_entity (HT.Param param_ent))
  else
    None

let substitute_inter_intra_backwards =
  substitute_inter_intra (`Backwards replace_backwards)

let substitute_inter_intra_forwards =
  substitute_inter_intra (`Forwards replace_forwards)

let equiv
    (any_constr_list_1 : any_constraint list)
    (any_constr_list_2 : any_constraint list) : bool =
  let only_intra_constr any_constr =
    let only_inter_ent (intra_constr : constraint_) :
        entity_ -> constraint_ option = function
      | Inter _ -> Some intra_constr
      | _ -> None
    in
    match any_constr with
    | HT.Intra intra_constr ->
      (match intra_constr with
      | Marks _ -> Some intra_constr
      | Static_key (_, _, ent, _, _) -> only_inter_ent intra_constr ent
      | Has_dynamic_key ent -> only_inter_ent intra_constr ent
      | _ -> None)
    | HT.Inter _ -> None
  in
  ConstraintSet.equal
    (ConstraintSet.of_list
       (List.filter_map ~f:only_intra_constr any_constr_list_1))
    (ConstraintSet.of_list
       (List.filter_map ~f:only_intra_constr any_constr_list_2))

let subsets (ent1 : entity_) (ent2 : entity_) : constraint_ =
  Subsets (ent1, ent2)
