(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type error = (Pos.t, Pos_or_decl.t) User_error.t [@@deriving eq, hash, show]

type finalized_error = (Pos.absolute, Pos.absolute) User_error.t
[@@deriving eq, show]

type format =
  | Context  (** Underlined references and color *)
  | Raw  (** Compact format with color but no references *)
  | Highlighted  (** Numbered and colored references *)
  | Plain  (** Verbose positions and no color *)
  | Extended
      (** Verbose context showing expressions, statements, hints, and declarations involved in error *)

(** Type representing the errors for a single file. *)
type per_file_errors

(** The type of collections of errors *)
type t [@@deriving eq, show]

type severity = User_error.severity

val iter : t -> f:(error -> unit) -> unit

module Error : sig
  type t = error

  val hash_for_saved_state : t -> Warnings_saved_state.ErrorHash.t
end

module ErrorSet : Stdlib.Set.S with type elt := error

module FinalizedErrorSet : Stdlib.Set.S with type elt := finalized_error

(** [t] is efficient for use inside hh_server or other places that compute errors,
which also supports incremental updates based on file.
But it should not be transferred to other processes such as [hh_client] since they
for instance won't know the Hhi path that was used, and hence can't print errors.
They should use [finalized_error list] instead. *)
val sort_and_finalize : t -> finalized_error list

module Parsing : Error_category.S

module Naming : Error_category.S

module NastCheck : Error_category.S

module Typing : Error_category.S

val read_lines : string -> string list

val num_digits : int -> int

val add_error : error -> unit

(* Error codes that can be suppressed in strict mode with a FIXME based on configuration. *)
val allowed_fixme_codes_strict : ISet.t ref

val is_strict_code : int -> bool

val set_allow_errors_in_default_path : bool -> unit

val get_disallowed_fixme_pos : (Pos.t -> int -> Pos.t option) ref

val get_ignore_pos : (Pos.t -> int -> Pos.t option) ref

val get_hh_fixme_pos : (Pos.t -> int -> Pos.t option) ref

val get_current_span : unit -> Pos.t

val is_suppressed : (Pos.t, 'a) User_error.t -> bool

val code_agnostic_fixme : bool ref

val convert_errors_to_string :
  ?include_filename:bool -> error list -> string list

val combining_sort : 'a list -> f:('a -> string) -> 'a list

val to_string : finalized_error -> string

(** Takes an error format option and gives back the error format that was passed
or the default formatter. *)
val format_or_default : format option -> format

(** Returns a summary string indicating things like how many errors were
  found, how many are displayed and how many were dropped.

  None is returned for [Raw] and [Plain] error formats. *)
val format_summary :
  format ->
  error_count:int ->
  warning_count:int ->
  dropped_count:int option ->
  max_errors:int option ->
  string option

val try_ : (unit -> 'a) -> (error -> 'a) -> 'a

val try_pred : fail:('a -> bool) -> (unit -> 'a) -> (unit -> 'a) -> 'a

val try_with_error : (unit -> 'a) -> (unit -> 'a) -> 'a

val try_with_result : (unit -> 'a) -> ('a -> error -> 'a) -> 'a

val run_and_check_for_errors : (unit -> 'a) -> 'a * bool

(** Return the list of errors caused by the function passed as parameter
    along with its result. *)
val do_ : ?apply_fixmes:bool -> ?drop_fixmed:bool -> (unit -> 'a) -> t * 'a

(** Return the list of errors caused by the function passed as parameter
    along with its result. *)
val do_with_context :
  ?drop_fixmed:bool -> Relative_path.t -> (unit -> 'a) -> t * 'a

val run_in_context : Relative_path.t -> (unit -> 'a) -> 'a

(** Turn on lazy decl mode for the duration of the closure.
    This runs without returning the original state,
    since we collect it later in do_with_lazy_decls_ *)
val run_in_decl_mode : (unit -> 'a) -> 'a

(* Run this function with span for the definition being checked.
 * This is used to check that the primary position for errors is not located
 * outside the span of the definition.
 *)
val run_with_span : Pos.t -> (unit -> 'a) -> 'a

(** ignore errors produced by function passed in argument. *)
val ignore_ : (unit -> 'res) -> 'res

val try_when :
  (unit -> 'res) -> if_error_and:(unit -> bool) -> then_:(error -> unit) -> 'res

val has_no_errors : (unit -> 'res) -> bool

val currently_has_errors : unit -> bool

val try_if_no_errors : (unit -> 'res) -> ('res -> 'res) -> 'res

val merge : t -> t -> t

val merge_into_current : t -> unit

(** [incremental_update ~old ~new_ ~rechecked] is for updating errors.
It starts with [old], removes every error in [rechecked],
then adds every error mentioned in [new_]. *)
val incremental_update : old:t -> new_:t -> rechecked:Relative_path.Set.t -> t

val empty : t

val is_empty : ?drop_fixmed:bool -> t -> bool

val count : ?drop_fixmed:bool -> t -> int

val get_error_list : ?drop_fixmed:bool -> t -> error list

val get_sorted_error_list : ?drop_fixmed:bool -> t -> error list

val as_map : t -> error list Relative_path.Map.t

val from_error_list : error list -> t

val drop_fixmed_errors :
  ('a, 'b) User_error.t list -> ('a, 'b) User_error.t list

val drop_fixmed_errors_in_files : t -> t

val from_file_error_list : (Relative_path.t * error) list -> t

val per_file_error_count : ?drop_fixmed:bool -> per_file_errors -> int

val get_file_errors :
  ?drop_fixmed:bool -> t -> Relative_path.t -> per_file_errors

val fold_per_file_errors :
  per_file_errors -> init:'acc -> f:('acc -> error -> 'acc) -> 'acc

val iter_error_list : ?drop_fixmed:bool -> (error -> unit) -> t -> unit

val fold_errors :
  ?drop_fixmed:bool ->
  t ->
  init:'a ->
  f:(Relative_path.t -> error -> 'a -> 'a) ->
  'a

val fold_errors_in :
  ?drop_fixmed:bool ->
  t ->
  file:Relative_path.t ->
  init:'a ->
  f:(error -> 'a -> 'a) ->
  'a

(** Get paths that have errors which haven't been HH_FIXME'd. *)
val get_failed_files : t -> Relative_path.Set.t

val as_telemetry_summary : t -> Telemetry.t

(** Get a [Telemetry.t] representation of errors.

Does not include fixme'd errors.

[limit] parameter to avoid blowing up our systems. *)
val as_telemetry :
  limit:int ->
  with_context_limit:int ->
  error_to_string:(finalized_error -> string) ->
  t ->
  Telemetry.t

val choose_code_opt : t -> int option

val sort : error list -> error list

(***************************************
 *                                     *
 *       Specific errors               *
 *                                     *
 ***************************************)

val internal_error : Pos.t -> string -> unit

val unimplemented_feature : Pos.t -> string -> unit

val experimental_feature : Pos.t -> string -> unit

(* The intention is to introduce invariant violations with `report_to_user`
   set to `false` initially. Then we observe and confirm that the invariant is
   not repeatedly violated. Only then, we set it to `true` in a subsequent
   release. This should prevent us from blocking users unexpectedly while
   gradually introducing signal for unexpected compiler states. *)
val invariant_violation :
  Pos.t -> Telemetry.t -> string -> report_to_user:bool -> unit

val exception_occurred : Pos.t -> Exception.t -> unit

val typechecker_timeout : Pos.t -> string -> int -> unit

val method_is_not_dynamically_callable :
  Pos.t ->
  string ->
  string ->
  bool ->
  (Pos_or_decl.t * string) option ->
  (Pos.t, Pos_or_decl.t) User_error.t option ->
  unit

val function_is_not_dynamically_callable : string -> error -> unit

val global_access_error :
  Error_codes.GlobalAccessCheck.t -> Pos.t -> string -> unit

val filter : t -> f:(Relative_path.t -> error -> bool) -> t

val count_errors_and_warnings : ('a, 'b) User_error.t list -> int * int

val filter_out_mergebase_warnings : Warnings_saved_state.t option -> t -> t

val filter_out_warnings : t -> t

val make_warning_saved_state : t -> Warnings_saved_state.t
