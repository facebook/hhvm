(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* Shared Search code between Fuzzy and Trie based searches *)

module type Searchable = sig
  type t
  val fuzzy_types : t list
  val compare_result_type : t -> t -> int
end

module Make(S : Searchable) = struct

  (* The results we'll return to the client *)
  type term = {
    name: string;
    pos: Pos.t;
    result_type: S.t;
  }

end
