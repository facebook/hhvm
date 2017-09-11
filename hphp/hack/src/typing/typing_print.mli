(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Pretty printing of types *)
(*****************************************************************************)

val error: 'a Typing_defs.ty_ -> string
val suggest: 'a Typing_defs.ty -> string
val full: Typing_env.env -> 'a Typing_defs.ty -> string
val full_rec: Typing_env.env -> int -> 'a Typing_defs.ty -> string
val full_strip_ns: Typing_env.env -> 'a Typing_defs.ty -> string
val debug: Typing_env.env -> 'a Typing_defs.ty -> string
val debug_with_tvars: Typing_env.env -> 'a Typing_defs.ty -> string
val class_: TypecheckerOptions.t -> Typing_defs.class_type -> string
val gconst: TypecheckerOptions.t -> Decl_heap.GConst.t -> string
val fun_: TypecheckerOptions.t -> Decl_heap.Fun.t -> string
val typedef: TypecheckerOptions.t -> Decl_heap.Typedef.t -> string
val constraints_for_type: Typing_env.env -> 'a Typing_defs.ty -> string option

(* Convert a type to a structured JSON value, as follows:
 * <prim> ::= "int" | "bool" | "float" | "string" | "num" | "arraykey"
 *         | "mixed" | "resource" | "void" | "noreturn"
 * <array-args> ::= [ <type> ] | [ <type>, <type> ]
 * <type> ::=
 *   mixed
 *     { "kind":"mixed" }
 *   this
 *     { "kind":"this" }
 *   any
 *     { "kind":"any" }
 *   Anonymous lambda types
 *     { "kind":"anon" }
 *   Primitives
 *     { "kind":"primitive", "name":<prim> }
 *   Arrays, with 0, 1 or 2 type arguments
 *     { "kind":"array", "args":<array-args> }
 *   Enums (with optional bound)
 *     { "kind":"enum", "name":<identifier> [,"as":<type>] }
 *   Newtype (with optional bound)
 *     { "kind":"newtype", "name":<identifier>, "args":<type-arguments>
 *        [,"as":<type>] }
 *   Nullable types, with singleton type argument
 *     { "kind":"nullable", "args":<type-arguments> }
 *   Classes (or interfaces, or traits) with 0 or more type arguments
 *     { "kind":"class", "name":<identifier>, "args":<type-arguments> }
 *   Generic parameters
 *     { "kind":"generic", "name":<identifier> }
 *   Tuples (and tuple-like arrays)
 *     { "kind":"tuple", "args":<type-arguments> }
 *   Shapes (and shape-like arrays) with fields
 *     { "kind":"shape", "fields":<fields> }
 *   Function types, with 0 or more arguments and possibly-void result type
 *     { "kind":"function", "args":<type-arguments>, "result":<type> }
 *   Unions of types, with 2 or more arguments
 *     { "kind":"union", "args":<type-arguments> }
 *   Type constant paths, with base type and list of identifiers
 *     { "kind":"path", "type":<type>, "path":<identifiers> [,"as":<type>] }
 *     Base type may be
 *         { "kind":"static" }
 *         { "kind":"this" }
 *         { "kind":"class", "name":<identifier>, "args":[]}
 *       Expression-dependent type
 *         { "kind":"expr" }
 *)
val to_json: Typing_env.env -> 'a Typing_defs.ty -> Hh_json.json
