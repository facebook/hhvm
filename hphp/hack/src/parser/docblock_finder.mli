(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * This is a simple data structure that allows querying for the docblock
 * given a line in the source code. Rough description:
 *
 * 1. Find the last comment preceding the line of the definition.
 *    We also make sure this doesn't overlap with the preceding definition.
 *    If the last comment is more than 1 line away, it is ignored.
 *
 * 2. If the last comment is a block-style comment (/* */) just return it.
 *
 * 3. Otherwise (if it is a line style comment //) attempt to merge it with
 *    preceding line comments, if they exist.
 *      NOTE: We also enforce that line comments must be on the definition's
 *            immediately preceding line.
 *)

type finder

val make_docblock_finder: ((Pos.t * Prim_defs.comment) list) -> finder

val find_docblock: finder -> ?last_line:int -> int -> string option

val find_single_docblock: ?tidy:bool -> Relative_path.t -> int -> string option

(** Find the last comment on `line` if it exists. *)
val find_inline_comment: finder -> int -> string option
