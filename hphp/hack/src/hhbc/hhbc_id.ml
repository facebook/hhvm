(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module SU = Hhbc_string_utils

let elaborate_id ns kind (_, id) =
  let (was_renamed, fq_id) = Namespaces.elaborate_id_impl ns kind id in
  (was_renamed, SU.strip_global_ns fq_id)

(* Class identifier, with namespace qualification if not global, but without
 * initial backslash.
 *)
module Class = struct
  type t = string

  let from_ast_name s = Hh_autoimport.normalize (SU.strip_global_ns s)

  let from_raw_string s = s

  let to_raw_string s = s

  let elaborate_id ns ((_, n) as id) =
    let ns =
      if SU.Xhp.is_xhp n then
        Namespace_env.empty
          ns.Namespace_env.ns_auto_ns_map
          ns.Namespace_env.ns_is_codegen
      else
        ns
    in
    let mangled_name = SU.Xhp.mangle n in
    let stripped_mangled_name = SU.strip_global_ns mangled_name in
    let (was_renamed, id) =
      elaborate_id ns Namespaces.ElaborateClass (fst id, mangled_name)
    in
    if was_renamed || mangled_name.[0] = '\\' then
      id
    else
      match Hh_autoimport.opt_normalize stripped_mangled_name with
      | None -> id
      | Some s -> s

  let to_unmangled_string s = SU.Xhp.unmangle s
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

  let elaborate_id ns id =
    let (_, fq_id) = elaborate_id ns Namespaces.ElaborateFun id in
    fq_id
end

module Const = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s

  let from_raw_string s = s

  let to_raw_string s = s

  let elaborate_id ns id =
    let (_, fq_id) = elaborate_id ns Namespaces.ElaborateConst id in
    fq_id
end
