(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Category : sig
  (** Care: each of these entries adds up to telemetry in [HackEventLogger.ProfileTypeCheck.get_stats],
  and we should keep the number small... 5 to 10 items here are fine, but more will be a problem. *)
  type t =
    | Disk_cat  (** This is counted every use of [Disk.cat] *)
    | Direct_decl_parse
        (** This is counted every use of [Direct_decl_utils.direct_decl_parse{_and_cache}].
        It often includes some [Disk_cat] time. *)
    | Decl_provider_get
        (** This measures every [Decl_provider.get_*] function. In cases where it's not already
        in the cache, this will also include calls to [Direct_decl_parse] hence also [Disk_cat],
        and also time to do decl-folding. *)
    | Ast_provider_get
        (** This measures every [Ast_provider.get_ast_with_error] call, for fetching the full ASTs of the files
        we're typechecking. It is inclusive of [Disk_cat] time.
        Note that this is *not* used for direct-decl-parsing. *)
    | Typing_toplevel
        (** This measures every [Typing_top_level] call. It will often include [Decl_provider_get]
        and hence [Direct_decl_parse] and [Disk_cat] for all the decls it needs.
        It doesn't include the call to [Ast_provider_get] for fetching the AST of the
        file we're typechecking; that's already been done prior. *)
end

module CategorySet : Set.S with type elt = Category.t

type time_in_sec = float

(** state is a global mutable variable, accumulating all counts *)
type t

(** reset will zero all counters, adjust the global mutable state,
    and return the previous state. You should 'restore_state' when done,
    in case your caller had been doing their own count.
    Categories from [enabled_categories] will be enabled
    and all others will be disabled. *)
val reset : unit -> t

(** restores global mutable state to what it was before you called 'reset' *)
val restore_state : t -> unit

val count : Category.t -> (unit -> 'a) -> 'a

val get_counters : unit -> Telemetry.t

val read_time : Category.t -> time_in_sec
