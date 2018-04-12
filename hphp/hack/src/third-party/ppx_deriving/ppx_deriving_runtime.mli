(** A module collecting all predefined OCaml types, exceptions and
    modules operating on them, so that ppx_deriving plugins operate
    in a well-defined environment. *)

(** {2 Predefined types} *)

(** The {!Predef} module is necessary in absence of a [type nonrec]
    construct. *)
module Predef : sig
  type _int = int
  type _char = char
  type _string = string
  type _float = float
  type _bool = bool (* = false | true *) (* see PR5936, GPR76, GPR234 *)
  type _unit = unit (* = () *)
  type _exn = exn
  type 'a _array = 'a array
  type 'a _list = 'a list (* = [] | 'a :: 'a list *)
  type 'a _option = 'a option = None | Some of 'a
  type _nativeint = nativeint
  type _int32 = int32
  type _int64 = int64
  type 'a _lazy_t = 'a lazy_t
  type _bytes = bytes
end

type int = Predef._int
type char = Predef._char
type string = Predef._string
type float = Predef._float
type bool = Predef._bool
type unit = Predef._unit
type exn = Predef._exn
type 'a array = 'a Predef._array
type 'a list = 'a Predef._list
type 'a option = 'a Predef._option = None | Some of 'a
type nativeint = Predef._nativeint
type int32 = Predef._int32
type int64 = Predef._int64
type 'a lazy_t = 'a Predef._lazy_t
type bytes = Predef._bytes

(** {2 Predefined modules}
    {3 Operations on predefined types} *)

module Pervasives : (module type of Pervasives with
  type fpclass = Pervasives.fpclass and
  type in_channel = Pervasives.in_channel and
  type out_channel = Pervasives.out_channel and
  type open_flag = Pervasives.open_flag and
  type 'a ref = 'a Pervasives.ref and
  type ('a, 'b, 'c, 'd, 'e, 'f) format6 = ('a, 'b, 'c, 'd, 'e, 'f) Pervasives.format6 and
  type ('a, 'b, 'c, 'd) format4 = ('a, 'b, 'c, 'd) Pervasives.format4 and
  type ('a, 'b, 'c) format = ('a, 'b, 'c) Pervasives.format)
include (module type of Pervasives with
  type fpclass = Pervasives.fpclass and
  type in_channel = Pervasives.in_channel and
  type out_channel = Pervasives.out_channel and
  type open_flag = Pervasives.open_flag and
  type 'a ref = 'a Pervasives.ref and
  type ('a, 'b, 'c, 'd, 'e, 'f) format6 = ('a, 'b, 'c, 'd, 'e, 'f) Pervasives.format6 and
  type ('a, 'b, 'c, 'd) format4 = ('a, 'b, 'c, 'd) Pervasives.format4 and
  type ('a, 'b, 'c) format = ('a, 'b, 'c) Pervasives.format)

module Char : (module type of Char)
module String : (module type of String)
module Printexc : (module type of Printexc with
  type raw_backtrace := Printexc.raw_backtrace and
  type backtrace_slot := Printexc.backtrace_slot and
  type location := Printexc.location)
module Array : (module type of Array)
module List : (module type of List)
module Nativeint : (module type of Nativeint)
module Int32 : (module type of Int32)
module Int64 : (module type of Int64)
module Lazy : (module type of Lazy)
module Bytes : (module type of Bytes)

(** {3 Data structures} *)

module Hashtbl : (module type of Hashtbl with
  type ('a, 'b) t := ('a, 'b) Hashtbl.t and
  type statistics := Hashtbl.statistics)
module Queue : (module type of Queue with
  type 'a t := 'a Queue.t)
module Stack : (module type of Stack with
  type 'a t := 'a Stack.t)
module Set : (module type of Set)
module Map : (module type of Map)
module Weak : (module type of Weak with
  type 'a t := 'a Weak.t)
module Buffer : (module type of Buffer with
  type t := Buffer.t)
module Result : sig
  type ('a, 'b) result = ('a, 'b) Pervasives.result =
    | Ok of 'a
    | Error of 'b
end

(** {3 Formatting} *)

module Printf : (module type of Printf)
module Format : (module type of Format with
  type formatter_out_functions := Format.formatter_out_functions and
  type formatter_tag_functions := Format.formatter_tag_functions and
  type formatter := Format.formatter)
