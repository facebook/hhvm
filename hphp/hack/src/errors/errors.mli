(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type error = (Pos.t, Pos_or_decl.t) User_error.t [@@deriving eq, show]

type finalized_error = (Pos.absolute, Pos.absolute) User_error.t
[@@deriving eq, show]

(* The analysis phase that the error is coming from. *)
type phase =
  | Init
  | Parsing
  | Naming
  | Decl
  | Typing
[@@deriving eq]

module PhaseMap : sig
  include Reordered_argument_collections.Map_S with type key = phase

  val pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit

  val show : (Format.formatter -> 'a -> unit) -> 'a t -> string
end

type format =
  | Context
  | Raw
  | Highlighted
  | Plain

(** Type representing the errors for a single file. *)
type per_file_errors

(** The type of collections of errors *)
type t [@@deriving eq, show]

module ErrorSet : Caml.Set.S with type elt := error

module FinalizedErrorSet : Caml.Set.S with type elt := finalized_error

val phases_up_to_excl : phase -> phase list

module Parsing : Error_category.S

module Naming : Error_category.S

module NastCheck : Error_category.S

module Typing : Error_category.S

val read_lines : string -> string list

val num_digits : int -> int

val add_error : error -> unit

val add_parsing_error : Parsing_error.t -> unit

val add_naming_error : Naming_error.t -> unit

val add_nast_check_error : Nast_check_error.t -> unit

val apply_error_from_reasons_callback :
  ?code:Typing.t ->
  ?claim:Pos.t Message.t Lazy.t ->
  ?reasons:Pos_or_decl.t Message.t list Lazy.t ->
  ?quickfixes:Pos.t Quickfix.t list ->
  Typing_error.Reasons_callback.t ->
  unit

val add_typing_error : Typing_error.t -> unit

val apply_callback_to_errors : t -> Typing_error.Reasons_callback.t -> unit

(* Error codes that can be suppressed in strict mode with a FIXME based on configuration. *)
val allowed_fixme_codes_strict : ISet.t ref

val report_pos_from_reason : bool ref

val is_strict_code : int -> bool

val set_allow_errors_in_default_path : bool -> unit

val is_hh_fixme : (Pos.t -> int -> bool) ref

val is_hh_fixme_disallowed : (Pos.t -> int -> bool) ref

val get_hh_fixme_pos : (Pos.t -> int -> Pos.t option) ref

val code_agnostic_fixme : bool ref

val phase_to_string : phase -> string

val phase_of_string : string -> phase option

val convert_errors_to_string :
  ?include_filename:bool -> error list -> string list

val combining_sort : 'a list -> f:('a -> string) -> 'a list

val to_string : finalized_error -> string

(** Prints a summary indicating things like how many errors were
  found, how many are displayed and how many were dropped. *)
val format_summary :
  format ->
  displayed_count:int ->
  dropped_count:int option ->
  max_errors:int option ->
  string option

val try_ : (unit -> 'a) -> (error -> 'a) -> 'a

val try_pred : fail:('a -> bool) -> (unit -> 'a) -> (unit -> 'a) -> 'a

val try_with_error : (unit -> 'a) -> (unit -> 'a) -> 'a

val try_with_result : (unit -> 'a) -> ('a -> error -> 'a) -> 'a

(** Return the list of errors caused by the function passed as parameter
    along with its result. *)
val do_ : ?apply_fixmes:bool -> ?drop_fixmed:bool -> (unit -> 'a) -> t * 'a

(** Return the list of errors caused by the function passed as parameter
    along with its result.
    The phase parameter determine the phase of the returned errors. *)
val do_with_context :
  ?drop_fixmed:bool -> Relative_path.t -> phase -> (unit -> 'a) -> t * 'a

val run_in_context : Relative_path.t -> phase -> (unit -> 'a) -> 'a

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

(** [incremental_update ~old ~new_ ~rechecked phase] is for updating errors.
It starts with [old], removes every error in [rechecked]+[phase],
then adds every error mentioned in [new_]. *)
val incremental_update :
  old:t -> new_:t -> rechecked:Relative_path.Set.t -> phase -> t

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

(** Default applied phase is Typing. *)
val from_file_error_list : ?phase:phase -> (Relative_path.t * error) list -> t

val per_file_error_count : ?drop_fixmed:bool -> per_file_errors -> int

val get_file_errors :
  ?drop_fixmed:bool -> t -> Relative_path.t -> per_file_errors

val iter_error_list : ?drop_fixmed:bool -> (error -> unit) -> t -> unit

val fold_errors :
  ?drop_fixmed:bool ->
  ?phase:phase ->
  t ->
  init:'a ->
  f:(Relative_path.t -> phase -> error -> 'a -> 'a) ->
  'a

val fold_errors_in :
  ?drop_fixmed:bool ->
  ?phase:phase ->
  t ->
  file:Relative_path.t ->
  init:'a ->
  f:(error -> 'a -> 'a) ->
  'a

val get_failed_files : t -> phase -> Relative_path.Set.t

val as_telemetry : t -> Telemetry.t

val choose_code_opt : t -> int option

val compare : error -> error -> int

val sort : error list -> error list

(***************************************
 *                                     *
 *       Specific errors               *
 *                                     *
 ***************************************)

val internal_error : Pos.t -> string -> unit

val unimplemented_feature : Pos.t -> string -> unit

val experimental_feature : Pos.t -> string -> unit

val ambiguous_inheritance :
  Pos_or_decl.t ->
  string ->
  string ->
  (Pos.t, Pos_or_decl.t) User_error.t ->
  Typing_error.Reasons_callback.t ->
  unit

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
