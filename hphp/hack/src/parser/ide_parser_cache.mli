(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Main on/off switch used to control using this cache. You should not be calling
 * get_ast/get_cst methods without calling enable() first *)
val enable : unit -> unit

val is_enabled : unit -> bool

(* To be on the safe side, we don't want ASTs from this cache to "leak" to non-IDE
 * scenarios (i.e. to be used to compute declarations that will be persisted in shared memory).
 * Activate/deactivate can be used to mark those "safe" sections, but it's up to the caller
 * to check if they really are safe. *)
val activate : unit -> unit

val deactivate : unit -> unit

(* Wraps a method in activate / deactivate *)
val with_ide_cache : (unit -> 'a) -> 'a

(* Cache key assigned to the file with this path and contents *)
val get_digest : Relative_path.t -> string -> string

(* Gets the AST from cache (or compute it if missing). The cache is keyed by hash
 * of path and content, and we assume that all callers will pass in ParserOptions
 * that result in equivalent AST.
 * Under this assumption, it's safe never invalidate this cache. *)
val get_ast : ParserOptions.t -> Relative_path.t -> string -> Parser_return.t

(* Optional version of get_ast that can be used on code paths that are shared between
 * "safe" and "unsafe" code paths *)
val get_ast_if_active :
  ParserOptions.t -> Relative_path.t -> string -> Parser_return.t option

val get_cst :
  Full_fidelity_source_text.t ->
  Full_fidelity_syntax_tree.WithSyntax(Full_fidelity_positioned_syntax).t
