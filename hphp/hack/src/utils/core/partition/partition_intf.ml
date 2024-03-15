(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type S = sig
  (* The set we are partitioning over. Sets are treated atomically,
     meaning we compute partitions without knowing the relationships
     between different sets in the partition. *)
  type set

  (** Abstract representation of a partition *)
  type t

  (** Operations on sets are represented in disjunctive normal form (DNF).
      This means it is a list of disjunctions (unions) that contain
      a list of conjunctions (intersections).

      (A & B) | C, where A, B, C are sets is represented as:
        [[A; B]; [C]]
      *)
  type dnf = set list list

  (* Smart Constructors for a partition*)

  (** A completely empty partition where both the left and right are empty *)
  val mk_bottom : t

  (** A partition where the set is assumed to be fully within the left side
     of the partition *)
  val mk_left : set -> t

  (** A partition where the set is assumed to be fully within the right side
     of the partition *)
  val mk_right : set -> t

  (** A partition that spans across the left and right partitions *)
  val mk_span : set -> t

  (** Unions two partitions together producing another partition *)
  val join : t -> t -> t

  (** Intersects two partitions together producing another partition *)
  val meet : t -> t -> t

  (* Accessors *)

  (** Returns the component of the partition that
      is definitely within the left side of the partition *)
  val left : t -> dnf

  (** Returns the component of the partition that is definitely
      spans across the left and right side of the partition *)
  val span : t -> dnf

  (** Returns the component of the partition that
      is definitely within the right side of the partition *)
  val right : t -> dnf

  module Infix_ops : sig
    val ( ||| ) : t -> t -> t

    val ( &&& ) : t -> t -> t
  end
end

module type Partition = sig
  module type S = S

  (* Constructs a partition representation over the given [AtomicSet] *)
  module Make (M : sig
    type t
  end) : S with type set := M.t
end
