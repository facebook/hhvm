(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type ('k, 'v) t
(** An LRU cache that's bounded in memory. When the size of all the elements in
the cache exceeds its maximum size, the cache evicts values until the size
falls below the maximum again.

The size of objects put into the cache is determined using
`Obj.reachable_words`. This may be an overestimate, particularly for the cases
where the pointed-to data is shared with other structures.

Only the sizes of values are tracked. The sizes of keys are not tracked, so they
don't count toward eviction.
*)

val make : max_size_in_words:int -> ('k, 'v) t
(** Construct a new cache which can store up to [max_size_in_words] words of
values. *)

val clear : ('k, 'v) t -> unit
(** Remove all entries from the cache. *)

val add : ('k, 'v) t -> key:'k -> value:'v -> unit
(** Add a [key]-[value] pair to the cache.

The cache is always resized to fit under the memory limit after any addition
operation. Under some circumstances, this could mean that the given [value] is
immediately evicted. (For example, if the [value] is greater than the maximum
size of the cache, then it must be evicted.) *)

val find_or_add : ('k, 'v) t -> key:'k -> default:(unit -> 'v) -> 'v
(** Find the element with the given [key] in the cache and return the
corresponding value. If the [key] is not present, calls [default] to calculate
its value, then [add]s it to the cache and returns that value.

The value is always guaranteed to be returned (whether by lookup or
calculation), although it may be evicted immediately from the cache (see note on
[add]). *)

val remove : ('k, 'v) t -> key:'k -> unit
(** Remove the entry with the given key from the cache. If the key is not
present, does nothing. *)
