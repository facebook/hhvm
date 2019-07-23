(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

module Syntax = Full_fidelity_editable_positioned_syntax

(**
 * Return the syntax of the following assignment:
 *
 * $bento_renamed_function_foo='bar';
 *
 * where foo is the first string and bar is the second string.
 *)
val synthesize_renaming_function_assignment_ast : string -> string -> Syntax.t

(**
 * Return the syntax of the following function declaration:
 *
 * function foo(mixed ...$x) {
 *  $GLOBALS['bento_renamed_function_foo'](...$x);
 * }
 *
 * where foo is replaced with the given string.
 *)
val synthesize_wrapper_function_declaration_ast : string -> Syntax.t
