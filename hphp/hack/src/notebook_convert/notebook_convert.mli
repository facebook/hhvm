(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Given the path to a notebook in .ipnyb (JSON) format,
 * print the result of converting to standard Hack or (on bad input)
 * return Exit_status.Input_error and print errors to stderr.
 * This conversion mainly involves:
 * - validation
 * - concatenating code cells (preserving cell information)
 * - Markdown cells converted to comments
 * - Moving top-level statements into a function body.
 *)
val notebook_to_hack : notebook_name:string -> Exit_status.t

(* Convert a Hack (.php) file (passed via stdin) back to a notebook *)
val hack_to_notebook : unit -> Exit_status.t
