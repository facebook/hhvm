(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

type search_result_type =
  | Class of Ast.class_kind
  | Method of bool * string
  | ClassVar of bool * string
  | Function
  | Typedef
  | Constant

type term = {
  name: string;
  pos: Pos.t;
  result_type: search_result_type;
}

module TMap : MapSig

type type_to_key_to_term_list = term list SMap.t TMap.t
type type_to_keyset = SSet.t TMap.t

module SearchKeyToTermMap : SharedMem.S with type t = type_to_key_to_term_list
module SearchKeys : SharedMem.S with type t = type_to_keyset

val process_term: Pos.t * string ->
           search_result_type ->
           type_to_keyset ->
           type_to_key_to_term_list ->
           type_to_keyset * type_to_key_to_term_list

val index_files: SSet.t -> unit

val query: string -> search_result_type option -> (term * int) list
