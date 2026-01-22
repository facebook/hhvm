(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type diagnostic = (Pos.t, Pos_or_decl.t) User_diagnostic.t
[@@deriving eq, hash, show]

type finalized_diagnostic = (Pos.absolute, Pos.absolute) User_diagnostic.t
[@@deriving eq, show]

type format =
  | Context  (** Underlined references and color *)
  | Raw  (** Compact format with color but no references *)
  | Highlighted  (** Numbered and colored references *)
  | Plain  (** Verbose positions and no color *)
  | Extended
      (** Verbose context showing expressions, statements, hints, and declarations involved in error *)

(** Type representing the errors for a single file. *)
type per_file_diagnostics

(** The type of collections of errors *)
type t [@@deriving eq, show]

type severity = User_diagnostic.severity

module Error : sig
  type t = diagnostic

  val hash_for_saved_state : t -> Warnings_saved_state.ErrorHash.t
end

module ErrorSet : Stdlib.Set.S with type elt := diagnostic

module FinalizedErrorSet : Stdlib.Set.S with type elt := finalized_diagnostic

(** [t] is efficient for use inside hh_server or other places that compute errors,
which also supports incremental updates based on file.
But it should not be transferred to other processes such as [hh_client] since they
for instance won't know the Hhi path that was used, and hence can't print errors.
They should use [finalized_diagnostic list] instead. *)
val sort_and_finalize : t -> finalized_diagnostic list

module Parsing : Error_category.S

module Naming : Error_category.S

module NastCheck : Error_category.S

module Typing : Error_category.S

val read_lines : string -> string list

val num_digits : int -> int

val add_diagnostic : diagnostic -> unit

(* Error codes that can be suppressed in strict mode with a FIXME based on configuration. *)
val allowed_fixme_codes_strict : ISet.t ref

val set_allow_errors_in_default_path : bool -> unit

val get_disallowed_fixme_pos : (Pos.t -> int -> Pos.t option) ref

val get_ignore_pos : (Pos.t -> int -> Pos.t option) ref

val get_hh_fixme_pos : (Pos.t -> int -> Pos.t option) ref

val get_current_span : unit -> Pos.t

val is_suppressed : (Pos.t, 'a) User_diagnostic.t -> bool

val code_agnostic_fixme : bool ref

val convert_errors_to_string :
  ?include_filename:bool -> diagnostic list -> string list

val combining_sort : 'a list -> f:('a -> string) -> 'a list

val to_string : finalized_diagnostic -> string

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

(** Run a computation; if it produces an error, call the error handler instead of
    adding the error to the error map. Warnings are added to the error map and
    don't trigger the error handler. *)
val try_ : (unit -> 'a) -> (diagnostic -> 'a) -> 'a

(** Run a computation; if it produces an error, call the continuation with both the
    result and the error. The continuation can inspect or transform the result.
    See {!try_} for warning behavior. *)
val try_with_result : (unit -> 'a) -> ('a -> diagnostic -> 'a) -> 'a

val run_and_check_for_errors : (unit -> 'a) -> 'a * bool

(** Return the list of errors caused by the function passed as parameter
    along with its result. *)
val do_ : ?drop_fixmed:bool -> (unit -> 'a) -> t * 'a

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

(** Like {!try_} but only calls the error handler if both an error occurs and
    the condition holds. Otherwise adds the error to the error map. *)
val try_when :
  (unit -> 'res) ->
  if_error_and:(unit -> bool) ->
  then_:(diagnostic -> unit) ->
  'res

val has_no_errors : (unit -> 'res) -> bool

val currently_has_errors : unit -> bool

val merge : t -> t -> t

val merge_into_current : t -> unit

(** [incremental_update ~old ~new_ ~rechecked] is for updating errors.
It starts with [old], removes every error in [rechecked],
then adds every error mentioned in [new_]. *)
val incremental_update : old:t -> new_:t -> rechecked:Relative_path.Set.t -> t

val empty : t

val is_empty : ?drop_fixmed:bool -> t -> bool

val count : ?drop_fixmed:bool -> t -> int

val get_diagnostic_list : ?drop_fixmed:bool -> t -> diagnostic list

val get_sorted_diagnostic_list : ?drop_fixmed:bool -> t -> diagnostic list

val as_map : t -> diagnostic list Relative_path.Map.t

val from_diagnostic_list : diagnostic list -> t

val drop_fixmed_errors :
  ('a, 'b) User_diagnostic.t list -> ('a, 'b) User_diagnostic.t list

val drop_fixmed_errors_in_files : t -> t

val from_file_diagnostic_list : (Relative_path.t * diagnostic) list -> t

val get_file_diagnostics :
  ?drop_fixmed:bool -> t -> Relative_path.t -> per_file_diagnostics

val fold_per_file_diagnostics :
  per_file_diagnostics -> init:'acc -> f:('acc -> diagnostic -> 'acc) -> 'acc

val iter_diagnostic_list :
  ?drop_fixmed:bool -> (diagnostic -> unit) -> t -> unit

val fold_errors :
  ?drop_fixmed:bool ->
  t ->
  init:'a ->
  f:(Relative_path.t -> diagnostic -> 'a -> 'a) ->
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
  error_to_string:(finalized_diagnostic -> string) ->
  t ->
  Telemetry.t

val choose_code_opt : t -> int option

val sort : diagnostic list -> diagnostic list

(***************************************
 *                                     *
 *       Specific errors               *
 *                                     *
 ***************************************)

val internal_error : Pos.t -> string -> unit

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
  (Pos.t, Pos_or_decl.t) User_diagnostic.t option ->
  unit

val function_is_not_dynamically_callable : string -> diagnostic -> unit

val global_access_error :
  Error_codes.GlobalAccessCheck.t -> Pos.t -> string -> unit

val filter : t -> f:(Relative_path.t -> diagnostic -> bool) -> t

val count_errors_and_warnings : ('a, 'b) User_diagnostic.t list -> int * int

val filter_out_mergebase_warnings : Warnings_saved_state.t option -> t -> t

val filter_out_warnings : t -> t

val make_warning_saved_state : t -> Warnings_saved_state.t
