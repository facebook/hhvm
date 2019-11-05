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

(* Class identifier, with namespace qualification if not global, but without
 * initial backslash.
 *)
module Class = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns @@ SU.Xhp.mangle s

  let from_raw_string s = s

  let to_raw_string s = s

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

  let from_ast_name s = SU.strip_global_ns s

  let from_raw_string s = s

  let to_raw_string s = s

  let add_suffix s suffix = s ^ suffix
end

module Const = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s

  let from_raw_string s = s

  let to_raw_string s = s
end

module Record = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s

  let from_raw_string s = s

  let to_raw_string s = s
end
