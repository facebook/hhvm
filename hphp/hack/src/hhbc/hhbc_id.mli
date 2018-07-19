(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

module Class : sig
  (* Class name as represented in assembler syntax. *)
  type t

  (* TODO: remove this and use elaborate_id everywhere *)
  val from_ast_name : string -> t

  (* For use only in assembler. Quotes have been removed already *)
  val from_raw_string : string -> t
  (* For use only at final stage of emitting assembler *)
  val to_raw_string : t -> string
  (* Without mangled XHP *)
  val to_unmangled_string : t -> string

  (* Given a namespace environment and a possibly-qualified identifier,
   * determine the HHAS representation of the identifier, with
   * namespace qualification but no initial backslash, and XHP
   * mangling. Also return the unqualified form as a string *)
  val elaborate_id : Namespace_env.env -> Ast.id -> t * string option
end

module Function : sig
  type t
  (* For use only in assembler. Quotes have been removed already *)
  val from_raw_string : string -> t
  (* For use only at final stage of emitting assembler *)
  val to_raw_string : t -> string
  (* Used to add suffixes for memoized functions *)
  val add_suffix : t -> string -> t
  val elaborate_id : Namespace_env.env -> Ast.id -> t * string option
  val elaborate_id_with_builtins : Namespace_env.env -> Ast.id -> t * string option

end

module Prop : sig
  type t
  val from_ast_name : string -> t
  val add_suffix : t -> string -> t
  val from_raw_string : string -> t
  val to_raw_string : t -> string
end

module Method : sig
  type t
  val from_ast_name : string -> t
  val add_suffix : t -> string -> t
  val from_raw_string : string -> t
  val to_raw_string : t -> string
end

module Const : sig
  type t
  (* From identifier in parse tree: remove initial backslash *)
  val from_ast_name : string -> t
  (* As used in `define('name')` and the assembler *)
  val from_raw_string : string -> t
  val to_raw_string : t -> string
  val elaborate_id : Namespace_env.env -> Ast.id -> t * string option * bool
end
