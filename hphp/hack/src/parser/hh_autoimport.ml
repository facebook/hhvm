(*
 * Copyright (c) 2017; Facebook; Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* Create map from name to alias *)
let add_alias m s = SMap.add s ("HH\\" ^ s) m

let alias_map =
  List.fold_left
    ~f:add_alias
    ~init:SMap.empty
    [
      "AsyncIterator";
      "AsyncKeyedIterator";
      "Traversable";
      "Container";
      "KeyedTraversable";
      "KeyedContainer";
      "Iterator";
      "KeyedIterator";
      "Iterable";
      "KeyedIterable";
      "Collection";
      "Vector";
      "Map";
      "Set";
      "Pair";
      "ImmVector";
      "ImmMap";
      "ImmSet";
      "InvariantException";
      "IMemoizeParam";
      "Shapes";
      "TypeStructureKind";
      "TypeStructure";
      "dict";
      "vec";
      "keyset";
      "varray";
      "darray";
      "Awaitable";
      "AsyncGenerator";
      "StaticWaitHandle";
      "WaitableWaitHandle";
      "ResumableWaitHandle";
      "AsyncFunctionWaitHandle";
      "AsyncGeneratorWaitHandle";
      "AwaitAllWaitHandle";
      "ConditionWaitHandle";
      "RescheduleWaitHandle";
      "SleepWaitHandle";
      "ExternalThreadEventWaitHandle";
      "bool";
      "int";
      "float";
      "string";
      "void";
      "num";
      "arraykey";
      "resource";
      "mixed";
      "noreturn";
      "this";
      "varray_or_darray";
      "vec_or_dict";
      "arraylike";
      "nonnull";
      "null";
      "nothing";
      "dynamic";
    ]

let opt_normalize s = SMap.get s alias_map

let normalize s = Option.value (opt_normalize s) ~default:s

let is_hh_autoimport s = SMap.mem s alias_map
