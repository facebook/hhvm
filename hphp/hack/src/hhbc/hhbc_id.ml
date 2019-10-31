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

let elaborate_id ns kind id =
  Namespaces.elaborate_id ns kind id |> snd |> SU.strip_global_ns

(* Class identifier, with namespace qualification if not global, but without
 * initial backslash.
 *)
module Class = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s

  let from_raw_string s = s

  let to_raw_string s = s

  let elaborate_id _ns (_, id_) = SU.strip_global_ns @@ SU.Xhp.mangle id_

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

  let elaborate_id _ns (_, id) = SU.strip_global_ns id
end

module Const = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s

  let from_raw_string s = s

  let to_raw_string s = s

  let elaborate_id ns id = elaborate_id ns Namespaces.ElaborateConst id
end

module Record = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s

  let from_raw_string s = s

  let to_raw_string s = s

  let elaborate_id _ns (_, id) = SU.strip_global_ns id
end
