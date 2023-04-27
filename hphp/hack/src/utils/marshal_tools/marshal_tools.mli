(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Invalid_Int_Size_Exception

exception Payload_Size_Too_Large_Exception

exception Malformed_Preamble_Exception

exception Writing_Preamble_Exception

exception Writing_Payload_Exception

exception Reading_Preamble_Exception

exception Reading_Payload_Exception

type remote_exception_data = {
  message: string;
  stack: string;
}
[@@deriving eq]

val of_exception : Exception.t -> remote_exception_data

(** Some say we should represent network communications failures with results,
not exceptions. Here we go for those who favor results... *)
type error =
  | Rpc_absent of Exception.t  (** socket isn't open to start with *)
  | Rpc_disconnected of Exception.t  (** closed while trying to read/write *)
  | Rpc_malformed of string * Utils.callstack  (** malformed packet *)
  | Rpc_remote_panic of remote_exception_data
      (** other party's unhandled exception *)

(** Turns an rpc_error into a detailed string suitable for debugging, maybe including stack trace *)
val error_to_verbose_string : error -> string

(** Writes a payload with preamble to a file descriptor.
    Returns the size of the marshalled payload.
    [timeout] is the timeout for [Timeout.select] which selects ready file descriptors.
    Unix write operations interrupted by signals are automatically restarted. *)
val to_fd_with_preamble :
  ?timeout:Timeout.t ->
  ?flags:Marshal.extern_flags list ->
  Unix.file_descr ->
  'a ->
  int

(** Reads a payload with preamble from a file descriptor.
    [timeout] is the timeout for [Timeout.select] which selects ready file descriptors.
    Unix read operations interrupted by signals are automatically restarted. *)
val from_fd_with_preamble : ?timeout:Timeout.t -> Unix.file_descr -> 'a

module type WRITER_READER = sig
  type 'a result

  type fd

  val return : 'a -> 'a result

  val fail : exn -> 'a result

  val ( >>= ) : 'a result -> ('a -> 'b result) -> 'b result

  val write :
    ?timeout:Timeout.t ->
    fd ->
    buffer:bytes ->
    offset:int ->
    size:int ->
    int result

  val read :
    ?timeout:Timeout.t ->
    fd ->
    buffer:bytes ->
    offset:int ->
    size:int ->
    int result

  val log : string -> unit
end

(** A "preamble" is a length 5 'bytes' that encodes a single integer up to size 2^31-1.
One of its bytes is a parity byte, to help safeguard against corruption. *)
val expected_preamble_size : int

(** "make_preamble n" will return a length 5 'bytes' that encodes the integer n. *)
val make_preamble : int -> bytes

(** "make_preamble n |> parse_preamble" will return the integer n. *)
val parse_preamble : bytes -> int

module MarshalToolsFunctor (WriterReader : WRITER_READER) : sig
  val to_fd_with_preamble :
    ?timeout:Timeout.t ->
    ?flags:Marshal.extern_flags list ->
    WriterReader.fd ->
    'a ->
    int WriterReader.result

  val from_fd_with_preamble :
    ?timeout:Timeout.t -> WriterReader.fd -> 'a WriterReader.result
end
