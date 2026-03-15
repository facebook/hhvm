(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

(**
 * Generate a code-like outline of a Hack file with line range annotations,
 * for AI agents. Helpful for determining which ranges of the file to read.
 *
 * Output format:
 *   class Name | lines 10-200
 *     function method_name | lines 12-50
 *     $property | line 55
 *     const CONST_NAME | line 60
 *
 * includes name and kind (class/function/etc.), omits private members
 *)
val outline : string -> string
