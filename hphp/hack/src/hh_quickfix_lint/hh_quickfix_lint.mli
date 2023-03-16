(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This interface file is for a module containing an executable with no public
   facing API. It is used for documentation and to make sure deadcode is caught
   by the compiler. *)

(* Given `hh --lint --json` output, this binary applies the quickfixes found in
   lints. The input is supplied either through `STDIN` or via a JSON file through
   the `--input-file` flag. In addition to handling the standard `hh --lint
   --json` output, the tool can also handle a JSON array of lints (which is
   equivalent to accessing the `"errors"` field of the `hh --lint --json`
   output).

   By default, the file output with the quickfix applied is printed to STDOUT.
   If the `--overwrite` flag is set, then the output overwrites the contents of
   file with the lint.

   The tool also takes a `--verbose` flag which prints out the positions for
   dropped lints for debugging purposes.

   LIMITATIONS:
   Currently, the tool cannot handle nested patches and drops the enclosing
   patch on the floor. However, the tool will report if the patch
   was dropped and for what reason.

   For reliability reasons, we double check that the "original" field is found
   at the starting offset of the quickfix. If not, we again drop the patch on
   the floor. *)
