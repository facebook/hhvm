(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core

module SU = Hhbc_string_utils

(* TODO: Remove this once we start reading this off of HHVM's config.hdf *)
let auto_namespace_map () =
  [ "Arrays", "HH\\Lib\\Arrays"
  ; "C", "HH\\Lib\\C"
  ; "Dict", "HH\\Lib\\Dict"
  ; "Keyset", "HH\\Lib\\Keyset"
  ; "Math", "HH\\Lib\\Math"
  ; "PHP", "HH\\Lib\\PHP"
  ; "Str", "HH\\Lib\\Str"
  ; "Vec", "HH\\Lib\\Vec"
  ] @
  Hhbc_options.(aliased_namespaces !compiler_options)

let elaborate_id ns kind id =
  let fully_qualified_id = snd (Namespaces.elaborate_id ns kind id) in
  let fully_qualified_id =
    Namespaces.renamespace_if_aliased
      ~reverse:true (auto_namespace_map ()) fully_qualified_id
  in
  let stripped_fully_qualified_id = SU.strip_global_ns fully_qualified_id in
  let clean_id = SU.strip_ns fully_qualified_id in
  let need_fallback =
    stripped_fully_qualified_id <> clean_id &&
    (String.contains stripped_fully_qualified_id '\\') &&
    not (String.contains (snd id) '\\')
  in
  stripped_fully_qualified_id, if need_fallback then Some clean_id else None

(* Class identifier, with namespace qualification if not global, but without
 * initial backslash.
 *)
module Class = struct
  type t = string

  let from_ast_name s =
    Hhbc_alias.normalize (SU.strip_global_ns s)
  let from_raw_string s = s
  let to_raw_string s = s
  let elaborate_id ns id =
    let mangled_name = SU.Xhp.mangle (snd id) in
    match Hhbc_alias.opt_normalize (SU.strip_global_ns mangled_name) with
    | None -> elaborate_id ns Namespaces.ElaborateClass (fst id, mangled_name)
    | Some s -> s, None

  let to_unmangled_string s =
    SU.Xhp.unmangle s
end

module Prop = struct
  type t = string

  let from_raw_string s = s
  let from_ast_name s = SU.strip_global_ns s
  let add_suffix s suffix = s ^ suffix
  let to_raw_string s = s
end

module Method = struct
  type t = string

  let from_raw_string s = s
  let from_ast_name s = SU.strip_global_ns s
  let add_suffix s suffix = s ^ suffix
  let to_raw_string s = s
end

module Function = struct
  type t = string

  (* See hphp/compiler/parser.cpp. *)
  let builtins_in_hh =
  [
    "fun";
    "meth_caller";
    "class_meth";
    "inst_meth";
    "invariant_callback_register";
    "invariant";
    "invariant_violation";
    "idx";
    "type_structure";
    "asio_get_current_context_idx";
    "asio_get_running_in_context";
    "asio_get_running";
    "xenon_get_data";
    "thread_memory_stats";
    "thread_mark_stack";
    "objprof_get_strings";
    "objprof_get_data";
    "objprof_get_paths";
    "heapgraph_create";
    "heapgraph_stats";
    "heapgraph_foreach_node";
    "heapgraph_foreach_edge";
    "heapgraph_foreach_root";
    "heapgraph_dfs_nodes";
    "heapgraph_dfs_edges";
    "heapgraph_node";
    "heapgraph_edge";
    "heapgraph_node_in_edges";
    "heapgraph_node_out_edges";
    "server_warmup_status";
    "dict";
    "vec";
    "keyset";
    "varray";
    "darray";
    "is_vec";
    "is_dict";
    "is_keyset";
    "is_varray_or_darray";
  ]

  let builtins_at_top = [
    "echo";
    "exit";
    "die";
    "func_get_args";
    "func_get_arg";
    "func_num_args"
  ]

  let has_hh_prefix s =
    let s = String.lowercase_ascii s in
    String_utils.string_starts_with s "hh\\"

  let is_hh_builtin s =
    let s = if has_hh_prefix s then String_utils.lstrip s "hh\\" else s in
    List.mem builtins_in_hh s

  let from_raw_string s = s
  let to_raw_string s = s
  let add_suffix s suffix = s ^ suffix
  let elaborate_id ns (_, s as id) =
    if is_hh_builtin s
    && (Emit_env.is_hh_file ()
    || Hhbc_options.enable_hiphop_syntax !Hhbc_options.compiler_options)
    then if has_hh_prefix s
          then s, None
          else SU.prefix_namespace "HH" s, Some s
    else if List.mem builtins_at_top s
      then s, None
      else elaborate_id ns Namespaces.ElaborateFun id

end

module Const = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s
  let from_raw_string s = s
  let to_raw_string s = s
  let elaborate_id ns id =
    let fq_id, backoff_id = elaborate_id ns Namespaces.ElaborateConst id in
    fq_id, backoff_id, String.contains (snd id) '\\'
end
