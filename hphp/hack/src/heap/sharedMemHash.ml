(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external get_hash : string -> int64 = "get_hash_ocaml"

let hash_string s = get_hash (Digest.string s)
