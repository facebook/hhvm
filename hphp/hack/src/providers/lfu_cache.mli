(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type size = int

module type Entry = sig
  (** The key-value pair type of the stored values. This is expected to be a
  GADT, which allows us to have a type-safe heterogeneous mapping from key type
  to value type.

  For example, for storing decls, we want to have different stored value types
  based on the key type. A [Fun_decl "foo"] should store a [Typing_defs.fun_elt]
  and a [Typedef_decl "foo"] should store a [Typing_defs.typedef_type]. (Note
  that functions and typedefs live in different namespaces.)

  In GADT syntax, this would be:

  ```
  type _ t =
    | Fun_decl : string -> Typing_defs.fun_elt t
    | Typedef_decl : string -> Typing_defs.typedef_decl t
  ```

  Then the following is well-typed:

  ```
  let foo_fun : Typing_defs.fun_elt option = Decl_cache.find_or_add
    cache
    ~key:(Fun_decl "foo")
    ~default:(fun () -> None)
  in
  let foo_typedef : Typing_defs.typedef_decl option = Decl_cache.find_or_add
    cache
    ~key:(Typedef_decl "foo")
    ~default:(fun () -> None)
  in
  *)
  type _ t

  (** Helper type alias. ['a Entry.key] should be read as "the key type of the
  entry key-value pair". *)
  type 'a key = 'a t

  (** Helper type alias. ['a Entry.value] should be read as "the value type of
  the entry key-value pair". *)
  type 'a value = 'a

  (** Get the size associated with a key-value pair. For example, you can
  measure its size in bytes. If the size is always [1], this causes the cache to
  act as a regular LRU cache. *)
  val get_size : key:'a key -> value:'a value -> size

  (** For logging/debugging *)
  val key_to_log_string : 'a key -> string
end

module Cache (Entry : Entry) : sig
  type t

  (** Construct a new cache which can store up to [max_size] of values. *)
  val make : max_size:size -> t

  (** Remove all entries from the cache. *)
  val clear : t -> unit

  (** Get the number of elements currently in the cache. *)
  val length : t -> int

  (** Add a [key]-[value] pair to the cache.

  The cache is always resized to fit under the memory limit after any addition
  operation. Under some circumstances, this could mean that the given [value] is
  immediately evicted. (For example, if the [value] is greater than the maximum
  size of the cache, then it must be evicted.) *)
  val add : t -> key:'a Entry.key -> value:'a Entry.value -> unit

  (** Find the element with the given [key] in the cache and return the
  corresponding value. If the [key] is not present, calls [default] to calculate
  its value, then [add]s it to the cache and returns that value. If [default]
  returns [None], then returns [None]; otherwise returns the computed value.

  Note that this could immediately evict the added value, if any was computed by
  [default] (see note on [add]). *)
  val find_or_add :
    t ->
    key:'a Entry.key ->
    default:(unit -> 'a Entry.value option) ->
    'a Entry.value option

  (* Check whether the item is present *)
  val find : t -> key:'a Entry.key -> 'a Entry.value option

  (** This uses [Entry.key_to_log_string] to get a list of all keys.
  Solely for debugging/logging. *)
  val keys_as_log_strings : t -> string list

  (** Remove the entry with the given key from the cache. If the key is not
  present, does nothing. *)
  val remove : t -> key:'a Entry.key -> unit

  (** Normally whenever you [add] or [find_or_add], if the cache is too large, then
  it will immediately do a collection. This function suppresses collections for the
  duration of the lambda; once the lambda terminates (even via an exception), it also
  checks whether the cache has become too large and if so does a collection then. *)
  val without_collections : t -> f:(unit -> 'a) -> 'a

  (** The cache keeps track of how long it's spent doing cache overhead and how big it is *)
  val get_telemetry : t -> key:string -> Telemetry.t -> Telemetry.t

  (** You can reset the timer. *)
  val reset_telemetry : t -> unit
end
