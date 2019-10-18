(*
 * Copyright (c) 2017; Facebook; Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* The typechecker has a different view of HH autoimporting than the compiler.
 * This type exists to track the unification of HH autoimporting between the
 * typechecker and the compiler, after which it will be deleted. *)
type autoimport_ns =
  | Global
  | HH

(**
 * Convenience function for maps that have been completely unified to autoimport
 * into the HH namespace.
 *)
let autoimport_map_of_list ids =
  List.fold_left ids ~init:SMap.empty ~f:(fun map id ->
      SMap.add id (HH, HH) map)

let autoimport_map_of_tuples ids =
  List.fold_left ids ~init:SMap.empty ~f:(fun map (id, typechecker_ns) ->
      SMap.add id (typechecker_ns, HH) map)

let types =
  autoimport_map_of_tuples
    [
      ("arraylike", HH);
      ("AsyncFunctionWaitHandle", HH);
      ("AsyncGenerator", Global);
      ("AsyncGeneratorWaitHandle", HH);
      ("AsyncIterator", Global);
      ("AsyncKeyedIterator", Global);
      ("Awaitable", Global);
      ("AwaitAllWaitHandle", HH);
      ("classname", HH);
      ("Collection", HH);
      ("ConditionWaitHandle", HH);
      ("Container", Global);
      ("darray", HH);
      ("dict", Global);
      ("ExternalThreadEventWaitHandle", HH);
      ("IMemoizeParam", HH);
      ("ImmMap", HH);
      ("ImmSet", HH);
      ("ImmVector", HH);
      ("InvariantException", Global);
      ("Iterable", Global);
      ("Iterator", Global);
      ("KeyedContainer", Global);
      ("KeyedIterable", Global);
      ("KeyedIterator", Global);
      ("KeyedTraversable", Global);
      ("keyset", Global);
      ("Map", HH);
      ("ObjprofObjectStats", HH);
      ("ObjprofPathsStats", HH);
      ("ObjprofStringStats", HH);
      ("Pair", HH);
      ("RescheduleWaitHandle", HH);
      ("ResumableWaitHandle", HH);
      ("Set", HH);
      ("Shapes", HH);
      ("SleepWaitHandle", HH);
      ("StaticWaitHandle", HH);
      ("this", Global);
      ("Traversable", Global);
      ("typename", HH);
      ("TypeStructure", HH);
      ("TypeStructureKind", HH);
      ("varray_or_darray", HH);
      ("varray", HH);
      ("vec_or_dict", HH);
      ("vec", Global);
      ("Vector", HH);
      ("WaitableWaitHandle", HH);
      ("XenonSample", HH);
    ]

let funcs =
  autoimport_map_of_list
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
      "objprof_get_strings";
      "server_warmup_status";
      "thread_mark_stack";
      "thread_memory_stats";
      "type_structure";
      "varray";
      "vec";
      "xenon_get_data";
    ]

let consts = autoimport_map_of_list ["Rx\\IS_ENABLED"]

let is_hh_autoimport s = SMap.mem s types

let lookup_type id =
  if Naming_special_names.Typehints.is_reserved_hh_name id then
    Some (Global, HH)
  else
    SMap.get id types

let reverse_type id =
  match String.chop_prefix ~prefix:"HH\\" id with
  | Some stripped_id when is_hh_autoimport stripped_id -> stripped_id
  | _ -> id

let lookup_func id = SMap.get id funcs

let lookup_const id = SMap.get id consts

let string_of_ns ns =
  match ns with
  | Global -> None
  | HH -> Some "HH"
