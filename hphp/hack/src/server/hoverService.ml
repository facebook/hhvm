(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type hover_info = {
  (** For fields and locals, this is the type. For method and function calls, it
      is the signature, including any inferred types for generics. *)
  info : string;
  doc_block : string option;
}

type result = hover_info list

let string_of_result { info; doc_block } =
  Printf.sprintf "{ info=\"%s\"; doc_block=%s }"
    info
    (match doc_block with | None -> "None" | Some db -> "\"" ^ db ^ "\"")
