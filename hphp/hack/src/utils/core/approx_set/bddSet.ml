(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include ApproxSet_intf

module Bdd (Atom : sig
  type t

  val compare : t -> t -> int
end) =
struct
  (** Represents a node in the BDD. [then_] is the branch taken when another set is
      a subset of [atom]. [else_] is the branch taken when not a subset. *)
  type node = {
    atom: Atom.t;
    then_: t;
    else_: t;
  }

  and t =
    | Bottom
    | Top
    | Select of node

  (* Operations supported by the BDD *)
  type op =
    | Meet
    | Join
    | Diff

  type order =
    | Same  (** The atoms are semantically the same *)
    | Lower  (** Lower atoms are closer to the root *)
    | Higher  (** Higher atoms are closer to the leaves *)

  let bdd_order a b =
    match Atom.compare a b with
    | 0 -> Same
    | x when x < 0 -> Lower
    | _ -> Higher

  let rec equal t1 t2 =
    if t1 == t2 then
      true
    else
      match (t1, t2) with
      | (Bottom, Bottom)
      | (Top, Top) ->
        true
      | (Select node1, Select node2) ->
        0 = Atom.compare node1.atom node2.atom
        && equal node1.then_ node2.then_
        && equal node1.else_ node2.else_
      | _ -> false

  let node atom ~then_ ~else_ =
    if equal then_ else_ then
      then_
    else
      Select { atom; then_; else_ }

  let atom = node ~then_:Top ~else_:Bottom

  let apply (op : op) : t -> t -> t =
    let interp op b1 b2 =
      match op with
      | Meet -> b1 && b2
      | Join -> b1 || b2
      | Diff -> b1 && not b2
    in
    let of_bool b =
      if b then
        Top
      else
        Bottom
    in
    let op_f_f = of_bool @@ interp op false false
    and op_f_t = of_bool @@ interp op false true
    and op_t_f = of_bool @@ interp op true false
    and op_t_t = of_bool @@ interp op true true in
    fun t1 t2 ->
      let rec aux t12 =
        match op with
        | Meet -> aux_meet t12
        | Join -> aux_join t12
        | Diff -> aux_diff t12
      and aux_meet ((t1, t2) as t12) =
        match (t1, t2) with
        | (Top, _) -> t2
        | (_, Top) -> t1
        | (Bottom, _)
        | (_, Bottom) ->
          Bottom
        | _ -> aux_gen t12
      and aux_join ((t1, t2) as t12) =
        match (t1, t2) with
        | (Bottom, _) -> t2
        | (_, Bottom) -> t1
        | (Top, _)
        | (_, Top) ->
          Top
        | _ -> aux_gen t12
      and aux_diff ((t1, t2) as t12) =
        match (t1, t2) with
        | (Bottom, _) -> Bottom
        | (_, Top) -> Bottom
        | (_, Bottom) -> t1
        | _ -> aux_gen t12
      and aux_gen (t1, t2) =
        match (t1, t2) with
        | (Bottom, Bottom) -> op_f_f
        | (Bottom, Top) -> op_f_t
        | (Top, Bottom) -> op_t_f
        | (Top, Top) -> op_t_t
        | (Select node1, (Bottom | Top)) ->
          (* Push terminal nodes to bottom of tree *)
          node
            node1.atom
            ~then_:(aux (node1.then_, t2))
            ~else_:(aux (node1.else_, t2))
        | ((Bottom | Top), Select node2) ->
          (* Push terminal nodes to bottom of tree *)
          node
            node2.atom
            ~then_:(aux (t1, node2.then_))
            ~else_:(aux (t1, node2.else_))
        | (Select node1, Select node2) ->
          (match bdd_order node1.atom node2.atom with
          | Same ->
            (* Atoms are the same, traverse edges of both *)
            node
              node1.atom
              ~then_:(aux (node1.then_, node2.then_))
              ~else_:(aux (node1.else_, node2.else_))
          | Lower ->
            (* node1 is lower, so push t2 closer to the
               leaves *)
            node
              node1.atom
              ~then_:(aux (node1.then_, t2))
              ~else_:(aux (node1.else_, t2))
          | Higher ->
            (* node1 is higher, so push t1 closer to the
               leaves *)
            node
              node2.atom
              ~then_:(aux (t1, node2.then_))
              ~else_:(aux (t1, node2.else_)))
      in
      aux (t1, t2)
end

module Make (Domain : OrderedDomainType) : S with module Domain := Domain =
struct
  include Bdd (Domain)

  let singleton = atom

  let union = apply Join

  let inter = apply Meet

  let diff = apply Diff

  let empty = Bottom

  exception
    RelationUnsat of {
      left: Domain.t;
      relation: SetRelation.t;
      right: Domain.t;
    }

  (** Checks the relationship between two bdds. The algorithm
      walks both bdds in similar fashion to [Bdd.apply]. When
      reaching the leaves we determine the [SetRelation.t] between
      them and return the result, merging as we go back up.

      What's key is that we utilize the [Domain.relation] function
      to perform simplifications as we traverse the tree. If [t1] is
      lower than [t2] we can simplify any [then_] edges in [t2] that are:
        * disjoint from [t1] when traversing [t1]'s [then_] edge
        * a subset of [t1] when traversing [t1]'s [else_] edge

      [is_sat] is used to break out early from the traversal if we
      can no longer meet the condition. When that occurs a [RelationUnsat]
      exception is raised

      Ideally we would employ memoization as we traverse to avoid
      repeated calls to [Domain.relation] or traversals as we simplify.
      Currently we do not to keep the implementation simpler. If
      performance becomes a concern then this is an optimization that
      can be done. *)
  let rec check_relation
      ?(is_sat : SetRelation.t -> bool = (fun _ -> true)) (t1 : t) (t2 : t) ~ctx
      : SetRelation.t =
    let check_relation = check_relation ~is_sat ~ctx in
    let merge (rel1 : SetRelation.t) (rel2 : SetRelation.t) : SetRelation.t =
      SetRelation.(
        make
          ~subset:(is_subset rel1 && is_subset rel2)
          ~superset:(is_superset rel1 && is_superset rel2)
          ~disjoint:(is_disjoint rel1 && is_disjoint rel2))
    in
    let rec erase_then_edges_if
        (ground : Domain.t) (bdd : t) ~(f : SetRelation.t -> bool) : t =
      let simplify = erase_then_edges_if ~f ground in
      match bdd with
      | Select { atom; then_; else_ } ->
        if f @@ Domain.relation ~ctx atom ground then
          simplify else_
        else
          node atom ~then_:(simplify then_) ~else_:(simplify else_)
      | Top -> Top
      | Bottom -> Bottom
    in
    let erase_then_edges_disjoint_from =
      erase_then_edges_if ~f:SetRelation.is_disjoint
    in
    let erase_then_edges_subset_of =
      erase_then_edges_if ~f:SetRelation.is_subset
    in
    let traverse_t1 ~(node1 : node) : SetRelation.t =
      merge
        (check_relation
           (erase_then_edges_disjoint_from node1.atom node1.then_)
           (erase_then_edges_disjoint_from node1.atom t2))
        (check_relation
           (erase_then_edges_subset_of node1.atom node1.else_)
           (erase_then_edges_subset_of node1.atom t2))
    in
    let traverse_t2 ~(node2 : node) : SetRelation.t =
      merge
        (check_relation
           (erase_then_edges_disjoint_from node2.atom t1)
           (erase_then_edges_disjoint_from node2.atom node2.then_))
        (check_relation
           (erase_then_edges_subset_of node2.atom t1)
           (erase_then_edges_subset_of node2.atom node2.else_))
    in
    match (t1, t2) with
    | (Bottom, Top) ->
      SetRelation.make ~subset:true ~superset:false ~disjoint:true
    | (Top, Bottom) ->
      SetRelation.make ~subset:false ~superset:true ~disjoint:true
    | (Bottom, Bottom) -> SetRelation.all
    | (Top, Top) -> SetRelation.equivalent
    | (Select node1, Select node2) ->
      let relation =
        match bdd_order node1.atom node2.atom with
        | Same ->
          merge
            (check_relation
               (erase_then_edges_disjoint_from node1.atom node1.then_)
               (erase_then_edges_disjoint_from node1.atom node2.then_))
            (check_relation
               (erase_then_edges_subset_of node1.atom node1.else_)
               (erase_then_edges_subset_of node1.atom node2.else_))
        | Lower -> traverse_t1 ~node1
        | Higher -> traverse_t2 ~node2
      in
      if not @@ is_sat relation then
        raise_notrace
        @@ RelationUnsat { left = node1.atom; relation; right = node2.atom }
      else
        relation
    | (Select node1, (Bottom | Top)) -> traverse_t1 ~node1
    | ((Bottom | Top), Select node2) -> traverse_t2 ~node2

  type disjoint =
    | Sat
    | Unsat of {
        left: Domain.t;
        relation: SetRelation.t;
        right: Domain.t;
      }

  let disjoint ctx a b =
    let is_sat = SetRelation.is_disjoint in
    try
      let relation = check_relation ~is_sat:SetRelation.is_disjoint ~ctx a b in
      if is_sat relation then
        Sat
      else
        let extract_atom = function
          | Select node -> node.atom
          | _ -> Domain.top
        in
        Unsat { left = extract_atom a; relation; right = extract_atom b }
    with
    | RelationUnsat { left; relation; right } -> Unsat { left; relation; right }

  let relate ctx a b = check_relation ~ctx a b

  let are_disjoint ctx set1 set2 =
    match disjoint ctx set1 set2 with
    | Sat -> true
    | Unsat _ -> false

  let of_list elt =
    List.fold_left (fun acc tag -> union acc @@ singleton tag) empty elt
end
