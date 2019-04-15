(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel

module SU = Hhbc_string_utils

let elaborate_id ns kind id =
  let autoimport = Emit_env.is_hh_syntax_enabled () in
  let was_renamed, fully_qualified_id =
    Namespaces.elaborate_id_impl ~autoimport ns kind (snd id) in
  let stripped_fully_qualified_id = SU.strip_global_ns fully_qualified_id in
  let clean_id = SU.strip_ns fully_qualified_id in
  let need_fallback =
    stripped_fully_qualified_id <> clean_id &&
    (String.contains stripped_fully_qualified_id '\\') &&
    not (String.contains (snd id) '\\')
  in
  was_renamed, stripped_fully_qualified_id, if need_fallback then Some clean_id else None

(* Class identifier, with namespace qualification if not global, but without
 * initial backslash.
 *)
module Class = struct
  type t = string

  let from_ast_name s =
    Hh_autoimport.normalize
      ~is_hack:(Emit_env.is_hh_syntax_enabled ())
      ~php7_scalar_types:(Hhbc_options.php7_scalar_types !Hhbc_options.compiler_options)
      (SU.strip_global_ns s)
  let from_raw_string s = s
  let to_raw_string s = s
  let elaborate_id ns ((_, n) as id) =
    let ns =
      if SU.Xhp.is_xhp n
      then Namespace_env.empty ns.Namespace_env.ns_popt
      else ns in
    let mangled_name = SU.Xhp.mangle n in
    let stripped_mangled_name = SU.strip_global_ns mangled_name in
    let was_renamed, id, _ =
      elaborate_id ns Namespaces.ElaborateClass (fst id, mangled_name) in
    if was_renamed || mangled_name.[0] = '\\'
    then id
    else
    match Hh_autoimport.opt_normalize
      ~is_hack:(Emit_env.is_hh_syntax_enabled ())
      ~php7_scalar_types:(Hhbc_options.php7_scalar_types !Hhbc_options.compiler_options)
      stripped_mangled_name with
    | None -> id
    | Some s -> s
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

  let has_hh_prefix s =
    let s = String.lowercase s in
    String_utils.string_starts_with s "hh\\"

  let is_hh_builtin s =
    let s = if has_hh_prefix s then String_utils.lstrip s "hh\\" else s in
    List.mem ~equal:(=) Namespaces.autoimport_funcs s

  let from_raw_string s = s
  let to_raw_string s = s
  let add_suffix s suffix = s ^ suffix

  let elaborate_id_with_builtins ns id =
    let _, fq_id, backoff_id = elaborate_id ns Namespaces.ElaborateFun id in
    match backoff_id with
      (* OK we are in a namespace so let's look at the backoff ID and see if
       * it's an HH\ or top-level function with implicit namespace.
       *)
    | Some id ->
      if List.mem ~equal:(=) Namespaces.autoimport_funcs id && (Emit_env.is_hh_syntax_enabled ())
      then SU.prefix_namespace "HH" id
      else fq_id
      (* Likewise for top-level, with no namespace *)
    | None ->
      if is_hh_builtin fq_id && (Emit_env.is_hh_syntax_enabled ())
      then
        if has_hh_prefix fq_id
        then fq_id
        else SU.prefix_namespace "HH" fq_id
      else fq_id

    let elaborate_id ns id =
      let _, fq_id, _ = elaborate_id ns Namespaces.ElaborateFun id in
      fq_id
end

module Const = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s
  let from_raw_string s = s
  let to_raw_string s = s
  let elaborate_id ns id =
    let _, fq_id, _ = elaborate_id ns Namespaces.ElaborateConst id in
    fq_id

end
