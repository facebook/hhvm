(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type notebook_number

(* `prefix` currently must match regex /^N[0-9]+$/ so we can
   * adjust it for function names, class names, and type names. *)
val create_notebook_number : string -> (notebook_number, string) result

(**
* Add a prefix to all declarations.
* `hack_source_code`: full contents of a Hack source file (not just a snippet)
* Example: n1234 can generate code with `function n1234_foo`, `class N1234Foo`, etc.
*)
val mangle : notebook_number -> string -> string

(**
* Remove a prefix from all declarations.
* `hack_source_code`: full contents of a Hack source file (not just a snippet)
* Example: with prefix `n1234`
* and code containing`function n1234_foo`, `class N1234Foo`, etc.
* we will remove the prefix and generate code with `function foo`, `class Foo`, etc.
*)
val unmangle : notebook_number -> string -> string
