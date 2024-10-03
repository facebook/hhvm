(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(** Specifies the domain that the set will range over. Individual
    members of the domain can be related to one another via the [relation]
    function. *)
module type DomainType = sig
  (** Type of atomic elements of the domain *)
  type t

  (** Contextual data that will be provided to [relation] when determining
      [SetRelation] between atomic elements of the domain *)
  type ctx

  (** Determines between atomic elements of the domain based on the given context.
      A valid implementation of [relation] must satisfy the following properties
      for all atoms A and B in the Domain:
        1. `SetRelation.is_subset   @@ relation ctx a b <=>
            SetRelation.is_superset @@ relation ctx b a`
        2. `SetRelation.is_superset @@ relation ctx a b <=>
            SetRelation.is_subset   @@ relation ctx b a`
        3. `SetRelation.is_disjoint @@ relation ctx a b <=>
            SetRelation.is_disjoint @@ relation ctx b a`

      We also require transitively as expected for the given set relations *)
  val relation : t -> ctx:ctx -> t -> SetRelation.t
end

(** An abstract representation of a set, designed specifically to determine if two
    sets are disjoint. Sets consist of atomic elements ([elt]) that can be joined
    using basic set operations like union, intersection and set difference. This is
    only an approximate set, because there are ways to construct sets where it isn't
    possible to determine definitively whether or not they are disjoint. In those cases
    the implementation will default to returning [Unsat]*)
module type S = sig
  (** The domain the set ranges over *)
  module Domain : DomainType

  (** Type of an instance of the approximate set *)
  type t

  val empty : t

  (** Create an approximate set from an atomic element in the domain *)
  val singleton : Domain.t -> t

  (** Set union *)
  val union : t -> t -> t

  (** Set intersection *)
  val inter : t -> t -> t

  (** Set difference. Note that in some cases we cannot determine precisely if
      two sets involving [diff] are disjoint. In these cases the result will
      be approximated to [Unsat] *)
  val diff : t -> t -> t

  val of_list : Domain.t list -> t

  (** The result of testing two sets for disjointness *)
  type disjoint =
    | Sat  (** The two sets are definitely disjoint *)
    | Unsat of {
        left: Domain.t;
        relation: SetRelation.t;
        right: Domain.t;
      }
        (** The two sets are not disjoint because of the relation between
            the given pair of [Domain.t]s *)

  (** Determines if a pair of sets are disjoint in the given [ctx].
      If the sets cannot definitively be proven to be disjoint, will return
      [Unsat] *)
  val disjoint : Domain.ctx -> t -> t -> disjoint

  val are_disjoint : Domain.ctx -> t -> t -> bool

  val relate : Domain.ctx -> t -> t -> SetRelation.t
end

module type ApproxSet = sig
  module type S = S

  (** Constructs an approximate set representation over the given [Domain] *)
  module Make (Domain : DomainType) : S with module Domain := Domain
end

(** A domain whose members can be ordered. The ordering is expected to satisfy
    the following conditions:
      1. There must exist a top element in the domain such that
         `SetRelation.is_superset @@ relation ctx (top ()) atom` is true for
         all atoms in the domain
      2. For any two atoms A and B, `SetRelation.is_superset @@ relation ctx a b`
         implies that `order a b <= 0`
      3. For any two atoms A and B, `order a b = 0` implies that
         `SetRelation.is_equivalent @@ relate ctx a b` *)
module type OrderedDomainType = sig
  include DomainType

  val compare : t -> t -> int

  val top : t
end

(** An implementation of the [ApproxSet] interface that utilizes a
    Binaray Decision Diagram as its underlined implementation. *)
module type BddSet = sig
  module type S = S

  (** Constructs an approximate set representation over the given [Domain] *)
  module Make (Domain : OrderedDomainType) : S with module Domain := Domain
end
