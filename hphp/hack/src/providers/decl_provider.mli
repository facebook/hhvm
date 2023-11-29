(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Provides decls from the configured backend, e.g. shared memory, local memory, service, etc. *)

type fun_key = string

type type_key = string

type gconst_key = string

type module_key = string

module Class : sig
  include module type of Typing_classes_heap.Api
end

type fun_decl = Typing_defs.fun_elt

type class_decl = Class.t

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.const_decl

type module_decl = Typing_defs.module_def_type

val get_fun :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  fun_key ->
  fun_decl Decl_entry.t

val get_class :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  type_key ->
  class_decl Decl_entry.t

(** Return the shallow declaration of the class with the given name if it is
present in the cache. Otherwise, compute it, store it in the cache, and
return it. *)
val get_shallow_class :
  Provider_context.t -> string -> Shallow_decl_defs.shallow_class option

val get_typedef :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  type_key ->
  typedef_decl Decl_entry.t

val get_gconst :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  gconst_key ->
  gconst_decl option

val get_module :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  module_key ->
  module_decl option

(** This assumes that [class_name] defines and overrides [method_name]
  and returns the method from an ancestor of [class_name] that would
  have been inherited by [class_name] had it not overridden it. *)
val get_overridden_method :
  Provider_context.t ->
  class_name:type_key ->
  method_name:string ->
  is_static:bool ->
  Typing_defs.class_elt Decl_entry.t

(** Return type for [is_this_def_the_winner] *)
type winner =
  | Winner  (** yes it is the winner *)
  | Loser_to of Pos.t  (** a different definition is the winner *)
  | Not_found  (** there is no winning definition for [name_type / name] *)

(** [is_this_def_the_winner ctx name_kind (pos, name)] judges whether the
the definition at [name / name_kind / pos] is deemed the "winner" in the naming table.
Normally when a symbol is defined only once, then it is the winner. But if a name is
defined multiple times (maybe differing in case), then only one of those definitions
is the winner, be they in the same file or different. Examples:
* [type tc=int; class tc {}] - these conflict, and only one may be deemed the winner
* [type t=int; type T=string;] - these conflict, and only one may be deemed the winner

Most other functions e.g. [Decl_provider.get_typedef] are not sensitive to winners,
and will happily return information about losers if so requested! For instance,
* [get_typedef "tc"] will return the typedef "type tc=int" even if "class tc {}" were the winner
* [get_typedef "T"] will return the typedef "type T=string" even if "type t=int" were the winner

This function returns [Winner] if the supplied definition is the winner,
or [Loser_to winner_pos] it it isn't the winner, or [Not_found] if there's no winner
for any capitalization of [name] in category [name_type] (nor, in the case of Class/Typedef,
in the other category].

It is an error to call this function with a [name_type / pos / name]
where [name_type / pos / different_capitalization_of_name] is the winner;
this will lead to an exception.

This function is fast in the common case where it returns [Winner].
The intended scenario is that (1) if the caller is iterating over all toplevel
definitions in an AST and calling this function on them, then by construction
it will never return [Not_found] or fail with an exception, (2) the only
time this function returns [Loser_to] is cases that report "duplicate name"
errors so it doesn't matter if they're a little slow.

We need to talk about the implementation strategy, because Decl_provider and
Naming_provider are leaky APIs and so the implementation of this function
is relevant to callers...
* In cases where it's needed to disambiguate winner/loser, this function consults the
reverse naming-table. Thus it depends for speed upon naming-table being cached.
* This function obtains the winning decl's position by reading the decl from Decl_provider
and all such decls are position-full. Thus it depends for speed upon decls being cached.
It is faster for instance than parsing ASTs, as is done by e.g.
[Naming_provider.get_type_full_pos_by_parsing_file]
* This function uses the horrid and slow [Naming_provider.get_{fun,type}_canon_name] in case
there wasn't a winner using the same capitalization as what was provided,
and so looks for the decl under that correct capitalization.

It would be great if the decl-provider could become more rigorous, so that
for instance [Decl_provider.get_class ctx name] would only return a decl if
it were the winner. Then we could do without the Naming-table cache.

It would be great to avoid [Naming_provider.get_{fun,type}_canon_name]. That will
only be possible in a future where Decl_provider supports case-insensitive decl lookups.
It's still okay if such lookups are slow - the only programs which cause us to do
these case-insensitive lookups will be programs with duplicate-name errors where
the duplicates differ in case.

If in future we decide not to use position-full decls, that will be easy. We'll
still need the decl-provider to tell us at least a filename, and we'll use
that filename to resolve winners/loosers that come from different files,
and we'll have this function take an [file_ast] parameter to resolve winners/loosers
that come from the same file. *)
val is_this_def_the_winner :
  Provider_context.t -> FileInfo.name_type -> Ast_defs.id -> winner

(** Internal helper used by [is_this_def_the_winner].
Exposed solely for testing in hh_single_decl; must not be used elsewhere *)
val get_pos_from_decl_of_winner_FOR_TESTS_ONLY :
  Provider_context.t -> FileInfo.name_type -> string -> Pos.t option

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit

val lookup_or_populate_class_cache :
  type_key ->
  (type_key -> Typing_classes_heap.class_t option) ->
  Typing_classes_heap.class_t option

val declare_folded_class_in_file_FOR_TESTS_ONLY :
  Provider_context.t -> type_key -> Decl_defs.decl_class_type
