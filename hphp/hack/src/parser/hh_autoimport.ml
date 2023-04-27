(*
 * Copyright (c) 2017; Facebook; Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** The names of autoimport types. *)
let types =
  [
    "AnyArray";
    "AsyncFunctionWaitHandle";
    "AsyncGenerator";
    "AsyncGeneratorWaitHandle";
    "AsyncIterator";
    "AsyncKeyedIterator";
    "Awaitable";
    "AwaitAllWaitHandle";
    "classname";
    "Collection";
    "ConcurrentWaitHandle";
    "ConditionWaitHandle";
    "Container";
    "darray";
    "dict";
    "ExternalThreadEventWaitHandle";
    "IMemoizeParam";
    "UNSAFESingletonMemoizeParam";
    "ImmMap";
    "ImmSet";
    "ImmVector";
    "InvariantException";
    "Iterable";
    "Iterator";
    "KeyedContainer";
    "KeyedIterable";
    "KeyedIterator";
    "KeyedTraversable";
    "keyset";
    "Map";
    "ObjprofObjectStats";
    "ObjprofPathsStats";
    "ObjprofStringStats";
    "Pair";
    "RescheduleWaitHandle";
    "ResumableWaitHandle";
    "Set";
    "Shapes";
    "SleepWaitHandle";
    "StaticWaitHandle";
    "supportdyn";
    "Traversable";
    "typename";
    "TypeStructure";
    "TypeStructureKind";
    "varray_or_darray";
    "varray";
    "vec_or_dict";
    "vec";
    "Vector";
    "WaitableWaitHandle";
    "XenonSample";
  ]

(** The names of autoimport functions. *)
let funcs =
  [
    "asio_get_current_context_idx";
    "asio_get_running_in_context";
    "asio_get_running";
    "class_meth";
    "darray";
    "dict";
    "fun";
    "heapgraph_create";
    "heapgraph_dfs_edges";
    "heapgraph_dfs_nodes";
    "heapgraph_edge";
    "heapgraph_foreach_edge";
    "heapgraph_foreach_node";
    "heapgraph_foreach_root";
    "heapgraph_node_in_edges";
    "heapgraph_node_out_edges";
    "heapgraph_node";
    "heapgraph_stats";
    "idx";
    "idx_readonly";
    "inst_meth";
    "invariant_callback_register";
    "invariant_violation";
    "invariant";
    "is_darray";
    "is_dict";
    "is_keyset";
    "is_varray";
    "is_vec";
    "keyset";
    "meth_caller";
    "objprof_get_data";
    "objprof_get_paths";
    "server_warmup_status";
    "thread_mark_stack";
    "thread_memory_stats";
    "type_structure";
    "type_structure_for_alias";
    "varray";
    "vec";
    "xenon_get_data";
  ]

(** The names of autoimport constants. *)
let consts = []

(** The names of autoimport namespaces. *)
let namespaces = ["Rx"]

(** Whether the given string is the name of an autoimport type. *)
let is_hh_autoimport =
  let h = HashSet.of_list types in
  (fun x -> HashSet.mem h x)

(** Strip any `HH\\` prefix from the provided string if the string
  is the name of a autoimport type. *)
let strip_HH_namespace_if_autoimport id =
  match String.chop_prefix ~prefix:"HH\\" id with
  | Some stripped_id when is_hh_autoimport stripped_id -> stripped_id
  | _ -> id
