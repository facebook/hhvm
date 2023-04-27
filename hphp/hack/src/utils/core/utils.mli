(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This `.mli` file was generated automatically. It may include extra
   definitions that should not actually be exposed to the caller. If you notice
   that this interface file is a poor interface, please take a few minutes to
   clean it up manually, and then delete this comment once the interface is in
   shape. *)

type callstack = Callstack of string [@@deriving show]

module Map : sig end

val spf : ('a, unit, string) format -> 'a

val print_endlinef : ('a, unit, string, unit) format4 -> 'a

val prerr_endlinef : ('a, unit, string, unit) format4 -> 'a

val pp_large_list :
  ?pp_sep:(Format.formatter -> unit -> unit) option ->
  ?max_items:int ->
  (Format.formatter -> 'a -> unit) ->
  Format.formatter ->
  'a list ->
  unit

val timestring : float -> string

val time : string -> float

val opt : ('a -> 'b -> 'a * 'c) -> 'a -> 'b option -> 'a * 'c option

val singleton_if : bool -> 'a -> 'a list

val wfold_left2 : ('a -> 'b -> 'c -> 'a) -> 'a -> 'b list -> 'c list -> 'a

val sl : string list -> string

val maybe : ('a -> 'b -> unit) -> 'a -> 'b option -> unit

val unsafe_opt_note : string -> 'a option -> 'a

val unsafe_opt : 'a option -> 'a

val try_with_stack : (unit -> 'a) -> ('a, Exception.t) result

val set_of_list : SSet.elt list -> SSet.t

(* Strip NS removes only the leading backslash *)
val strip_ns : string -> string

(* Strip XHP removes only the leading colon *)
val strip_xhp_ns : string -> string

(* Strip Both removes either leading backslash and colon, or both *)
val strip_both_ns : string -> string

val strip_hh_lib_ns : string -> string

(* Strip All removes all backslash-based namespaces, but does nothing to XHP *)
val strip_all_ns : string -> string

(** A\B\C -> \A\B\C *)
val add_ns : string -> string

(** A:B:C -> :A:B:C *)
val add_xhp_ns : string -> string

val split_ns_from_name : string -> string * string

val expand_namespace : (string * string) list -> string -> string

val iter2_shortest : ('a -> 'b -> 'c) -> 'a list -> 'b list -> unit

val compose : ('a -> 'b) -> ('c -> 'a) -> 'c -> 'b

module With_complete_flag : sig
  type 'a t = {
    is_complete: bool;
    value: 'a;
  }
end

val try_finally : f:(unit -> 'a) -> finally:(unit -> unit) -> 'a

val with_context :
  enter:(unit -> unit) -> exit:(unit -> unit) -> do_:(unit -> 'a) -> 'a

val assert_false_log_backtrace : string option -> 'a

val infimum : 'a array -> 'b -> ('a -> 'b -> int) -> int option

val unwrap_snd : 'a * 'b option -> ('a * 'b) option
