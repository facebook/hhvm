(**
 * Copyright (c) 2017; Facebook; Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* The auto alias map for Hack types. This is adapted from the
 * Parser::AutoAliasMap in hphp/compiler/parser/{parser.cpp,parser.h}
 *)

open Core_kernel

type alias =
| HH_ONLY_TYPE of string
| SCALAR_TYPE of string
| HH_ALIAS of string * string

(* Create map from name to (alias, is_php7_scalar_type) *)
let add_alias m a =
  let add k v =
    SMap.add (String.lowercase k) v m in
  match a with
  | HH_ONLY_TYPE s ->
    add s ("HH\\" ^ s, false)
  | SCALAR_TYPE s ->
    add s ("HH\\" ^ s, true)
  | HH_ALIAS (s, alias) ->
    add s (alias, false)

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
  SCALAR_TYPE("void");

  HH_ONLY_TYPE("num");
  HH_ONLY_TYPE("arraykey");
  HH_ONLY_TYPE("resource");
  HH_ONLY_TYPE("mixed");
  HH_ONLY_TYPE("noreturn");
  HH_ONLY_TYPE("this");
  HH_ONLY_TYPE("varray_or_darray");
  HH_ONLY_TYPE("vec_or_dict");
  HH_ONLY_TYPE("arraylike");
  HH_ONLY_TYPE("nonnull");
  HH_ONLY_TYPE("null");
  HH_ONLY_TYPE("nothing");

  HH_ALIAS("classname", "string");
  HH_ALIAS("typename", "string");
  HH_ALIAS("boolean", "bool");
  HH_ALIAS("integer", "int");
  HH_ALIAS("double", "float");
  HH_ALIAS("real", "float");
  HH_ALIAS("dynamic", "mixed");

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

let rec normalize ~is_hack ~php7_scalar_types s =
  if not (is_hack || php7_scalar_types)
  then s
  else
  match SMap.get (String.lowercase s) alias_map with
  | None -> s
  | Some (a, is_scalar_type) ->
    if is_hack || is_scalar_type
    then normalize ~is_hack ~php7_scalar_types a
    else s

let opt_normalize ~is_hack ~php7_scalar_types s =
  match String.lowercase s with
  | "callable" -> Some "callable"
  | "array" -> Some "array"
  | s ->
    if not (is_hack || php7_scalar_types)
    then None
    else
    match SMap.get s alias_map with
    | None -> None
    | Some (a, is_scalar_type) ->
      if is_hack || is_scalar_type
      then Some (normalize ~is_hack ~php7_scalar_types a)
      else None

let is_hh_autoimport s = SMap.mem (String.lowercase s) alias_map
