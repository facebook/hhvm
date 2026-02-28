(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Print errors as something like (with fancy coloring based on severity):

      warning: Warn[12001] This expression is likely always `false`
      --> path/to/hack_file.php
      15 | function h(?int $a, ?string $b): bool {
      |             ^^^ This is an int
      |                      ^^^^^^ This is a string
      17 |   return $a === $b;
      |          ^^^^^^^^^
*)
val to_string :
  ?claim_color:Tty.raw_color -> Diagnostics.finalized_diagnostic -> string
