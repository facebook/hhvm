(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

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
  let need_fallback =
    ns.Namespace_env.ns_name <> None &&
    not (String.contains (snd id) '\\') in
  if need_fallback
  then SU.strip_global_ns fully_qualified_id, Some (snd id)
  else SU.strip_global_ns fully_qualified_id, None

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
    | None -> elaborate_id ns Ast_defs.NSClass (fst id, mangled_name)
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

  let from_raw_string s = s
  let to_raw_string s = s
  let add_suffix s suffix = s ^ suffix
  let elaborate_id ns (_, s as id) =
    (* TODO: maintain set of builtin names *)
    match s with
    | "invariant_violation"
    | "invariant"
    | "type_structure"
    | "fun"
    | "meth_caller"
    | "class_meth"
    | "inst_meth" -> SU.prefix_namespace "HH" s, Some s
    | "func_get_args" | "func_num_args" ->
      s, None
    | _ ->
      elaborate_id ns Ast_defs.NSFun id

end

module Const = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s
  let from_raw_string s = s
  let to_raw_string s = s
  let elaborate_id ns id =
    let fq_id, backoff_id = elaborate_id ns Ast_defs.NSConst id in
    fq_id, backoff_id, String.contains (snd id) '\\'
end
