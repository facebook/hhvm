module Predef = struct
  type _int = int
  type _char = char
  type _string = string
  type _float = float
  type _bool = bool
  type _unit = unit
  type _exn = exn
  type 'a _array = 'a array
  type 'a _list = 'a list
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

module Pervasives = Pervasives
module Char = Char
module String = String
module Printexc = Printexc
module Array = Array
module List = List
module Nativeint = Nativeint
module Int32 = Int32
module Int64 = Int64
module Lazy = Lazy
module Bytes = Bytes

module Hashtbl = Hashtbl
module Queue = Queue
module Stack = Stack
module Set = Set
module Map = Map
module Weak = Weak

module Printf = Printf
module Format = Format
module Buffer = Buffer
module Result = struct
  type ('a, 'b) result = ('a, 'b) Pervasives.result =
    | Ok of 'a
    | Error of 'b
end

include Pervasives
