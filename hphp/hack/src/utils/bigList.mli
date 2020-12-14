(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A BigList is like a list but with potentially huge contents.
It keeps a "length" integer, so that BigList.length can be O(1) rather than O(t). *)
type 'a t

val empty : 'a t

val cons : 'a -> 'a t -> 'a t

(* This is O(t) as it traverses input list to calculate length. *)
val create : 'a list -> 'a t

(* This is O(1). It simply discards its memory of the length. *)
val as_list : 'a t -> 'a list

val is_empty : 'a t -> bool

(* This is O(1). *)
val length : 'a t -> int

(* This is O(1). If the list is empty, returns None; otherwise returns Some(hd,tl). *)
val decons : 'a t -> ('a * 'a t) option

(** This is O(t).
    [filter l ~f] returns all the elements of the list [l] that satisfy the predicate [p].
    The order of the elements in the input list is preserved. *)
val filter : 'a t -> f:('a -> bool) -> 'a t

(** This is O(t).
    [map f [a1; ...; an]] applies function [f] to [a1], [a2], ..., [an], in order,
    and builds the list [[f a1; ...; f an]] with the results returned by [f]. *)
val map : 'a t -> f:('a -> 'b) -> 'b t

(** [append small big] appends a small list in front of a BigList, without traversing the BigList. *)
val append : 'a list -> 'a t -> 'a t

(** [rev_appends small big] a small list in front of a BigList, without traversing the BigList.
    It is like BigList.append (List.rev small) big, but more efficient on the small list. *)
val rev_append : 'a list -> 'a t -> 'a t

(** List reversal. This is O(t). *)
val rev : 'a t -> 'a t

(** [split t n] splits of the first n elements of t, without traversing
    the remainder of t. It is O(n). *)
val split_n : 'a t -> int -> 'a list * 'a t

val pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit
