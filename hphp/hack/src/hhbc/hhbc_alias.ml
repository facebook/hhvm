(**
 * Copyright (c) 2017; Facebook; Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* The auto alias map for Hack types. This is adapted from the
 * Parser::AutoAliasMap in hphp/compiler/parser/{parser.cpp,parser.h}
 *)

open Core
open Hhbc_string_utils

type alias =
| HH_ONLY_TYPE of string
| SCALAR_TYPE of string
| HH_ALIAS of string * string

(* TODO: distinguish these kinds of alias, as in HHVM *)
let add_alias m a =
  let add k v =
    SMap.add (String.lowercase_ascii k) v m in
  match a with
  | HH_ONLY_TYPE s -> add s (prefix_namespace "HH" s)
  | SCALAR_TYPE s -> add s (prefix_namespace "HH" s)
  | HH_ALIAS (s, alias) -> add s alias

let alias_map = List.fold_left ~f:add_alias ~init:SMap.empty
[
  HH_ONLY_TYPE("AsyncIterator");
  HH_ONLY_TYPE("AsyncKeyedIterator");
  HH_ONLY_TYPE("Traversable");
  HH_ONLY_TYPE("Container");
  HH_ONLY_TYPE("KeyedTraversable");
  HH_ONLY_TYPE("KeyedContainer");
  HH_ONLY_TYPE("Iterator");
  HH_ONLY_TYPE("KeyedIterator");
  HH_ONLY_TYPE("Iterable");
  HH_ONLY_TYPE("KeyedIterable");
  HH_ONLY_TYPE("Collection");
  HH_ONLY_TYPE("Vector");
  HH_ONLY_TYPE("Map");
  HH_ONLY_TYPE("Set");
  HH_ONLY_TYPE("Pair");
  HH_ONLY_TYPE("ImmVector");
  HH_ONLY_TYPE("ImmMap");
  HH_ONLY_TYPE("ImmSet");
  HH_ONLY_TYPE("InvariantException");
  HH_ONLY_TYPE("IMemoizeParam");
  HH_ONLY_TYPE("Shapes");
  HH_ONLY_TYPE("TypeStructureKind");
  HH_ONLY_TYPE("TypeStructure");
  HH_ONLY_TYPE("dict");
  HH_ONLY_TYPE("vec");
  HH_ONLY_TYPE("keyset");
  HH_ONLY_TYPE("varray");
  HH_ONLY_TYPE("darray");

  HH_ONLY_TYPE("Awaitable");
  HH_ONLY_TYPE("AsyncGenerator");
  HH_ONLY_TYPE("WaitHandle");
  HH_ONLY_TYPE("StaticWaitHandle");
  HH_ONLY_TYPE("WaitableWaitHandle");
  HH_ONLY_TYPE("ResumableWaitHandle");
  HH_ONLY_TYPE("AsyncFunctionWaitHandle");
  HH_ONLY_TYPE("AsyncGeneratorWaitHandle");
  HH_ONLY_TYPE("AwaitAllWaitHandle");
  HH_ONLY_TYPE("ConditionWaitHandle");
  HH_ONLY_TYPE("RescheduleWaitHandle");
  HH_ONLY_TYPE("SleepWaitHandle");
  HH_ONLY_TYPE("ExternalThreadEventWaitHandle");

  SCALAR_TYPE("bool");
  SCALAR_TYPE("int");
  SCALAR_TYPE("float");
  SCALAR_TYPE("string");

  HH_ONLY_TYPE("num");
  HH_ONLY_TYPE("arraykey");
  HH_ONLY_TYPE("resource");
  HH_ONLY_TYPE("mixed");
  HH_ONLY_TYPE("noreturn");
  HH_ONLY_TYPE("void");
  HH_ONLY_TYPE("this");
  HH_ONLY_TYPE("varray_or_darray");

  HH_ALIAS("classname", "string");
  HH_ALIAS("typename", "string");
  HH_ALIAS("boolean", "bool");
  HH_ALIAS("integer", "int");
  HH_ALIAS("double", "float");
  HH_ALIAS("real", "float");
(*
  PHP7_TYPE("Throwable"; PHP7_EngineExceptions);
  PHP7_TYPE("Error"; PHP7_EngineExceptions);
  PHP7_TYPE("ArithmeticError"; PHP7_EngineExceptions);
  PHP7_TYPE("AssertionError"; PHP7_EngineExceptions);
  PHP7_TYPE("DivisionByZeroError"; PHP7_EngineExceptions);
  PHP7_TYPE("ParseError"; PHP7_EngineExceptions);
  PHP7_TYPE("TypeError"; PHP7_EngineExceptions);
  *)
]

let rec normalize s =
  match SMap.get (String.lowercase_ascii s) alias_map with
  | None -> s
  | Some a -> normalize a

let opt_normalize s =
  match String.lowercase_ascii s with
  | "callable" -> Some "callable"
  | "array" -> Some "array"
  | s -> Option.map ~f:normalize (SMap.get s alias_map)
