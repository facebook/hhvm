(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module to filter a list of errors in order to implement the
  warnings CLI flags like -Wall, -Wnone, -W, -Wno, etc. *)

module Code = Error_codes.Warning

type switch =
  | WAll  (** -Wall *)
  | WNone  (** -Wnone *)
  | Code_on of Code.t  (** -W 12001 *)
  | Code_off of Code.t  (** -Wno 12001 *)
  | Ignored_files of Str.regexp  (** -Wignored-files <regexp> *)
  | Generated_files_on  (** -Wgenerated *)

module Filter : sig
  type t

  (** [default_all] controls wether an empty list of switches is equivalent to [WAll]
    as opposed to [WNone] *)
  val make :
    default_all:bool -> generated_files:Str.regexp list -> switch list -> t
end

val filter :
  Filter.t ->
  Diagnostics.finalized_diagnostic list ->
  Diagnostics.finalized_diagnostic list

val filter_with_hash :
  Filter.t ->
  (Diagnostics.finalized_diagnostic * int) list ->
  (Diagnostics.finalized_diagnostic * int) list

val filter_rel : Filter.t -> Diagnostics.t -> Diagnostics.t
