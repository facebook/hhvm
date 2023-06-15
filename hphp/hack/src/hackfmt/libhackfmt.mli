(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Format an entire file. *)
val format_tree : ?config:Format_env.t -> Noformat.SyntaxTree.t -> string

val format_doc : Format_env.t -> Doc.t -> string

val format_doc_unbroken : Format_env.t -> Doc.t -> string

(** Format a node at the given offset.
  *
  * Finds the node which is the direct parent of the token at the given byte
  * offset and formats a range containing that node which ends at the given
  * offset. The range that was formatted is returned (as a pair of 0-based byte
  * offsets in the original file) along with the formatted text.
  *
  * The provided offset must point to the last byte in a token. If not, an
  * invalid_arg exception will be raised.
  *
  * Designed to be suitable for as-you-type-formatting. *)
val format_at_offset :
  ?config:Format_env.t -> Noformat.SyntaxTree.t -> int -> (int * int) * string

(** Return the source of the entire file with the given intervals formatted.
  *
  * The intervals are a list of half-open intervals of 1-based line numbers.
  * They are permitted to overlap. *)
val format_intervals :
  ?config:Format_env.t -> (int * int) list -> Noformat.SyntaxTree.t -> string

(** Format a given range in a file.
 *
 * The range is a half-open interval of byte offsets into the file.
 *
 * If the range boundaries would bisect a token, the entire token will appear in
 * the formatted output.
 *
 * If the first token in the range would have indentation preceding it in the
 * full formatted file, the leading indentation will be included in the output.
 *
 * If the last token in the range would have a trailing newline in the full
 * formatted file, the trailing newline will be included in the output.
 *
 * Non-indentation space characters are not included at the beginning or end of
 * the formatted output (unless they are in a comment or string literal). *)
val format_range :
  ?config:Format_env.t -> Interval.t -> Noformat.SyntaxTree.t -> string

val env_from_config : Format_env.t option -> Format_env.t
