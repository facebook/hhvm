(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

(** Find the nearest directory containing [.hhconfig], starting at the given
    directory and searching its ancestors up to Rust's traversal limit.
    This is a best-effort lookup: all discovery failures return [None], so callers
    can fall back to another resolver without distinguishing failure reasons. *)
external guess_root : string -> string option = "hh_guess_repo_root"
