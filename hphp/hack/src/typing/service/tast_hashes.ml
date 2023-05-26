(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type hash = Hash.hash_value

let yojson_of_hash = yojson_of_int

type by_names = {
  fun_tast_hashes: hash SMap.t; [@yojson_drop_if SMap.is_empty]
  class_tast_hashes: hash SMap.t; [@yojson_drop_if SMap.is_empty]
  typedef_tast_hashes: hash SMap.t; [@yojson_drop_if SMap.is_empty]
  gconst_tast_hashes: hash SMap.t; [@yojson_drop_if SMap.is_empty]
  module_tast_hashes: hash SMap.t; [@yojson_drop_if SMap.is_empty]
}
[@@deriving yojson_of]

type t = by_names Relative_path.Map.t [@@deriving yojson_of]

let hash_tasts
    { Tast.fun_tasts; class_tasts; typedef_tasts; gconst_tasts; module_tasts } :
    by_names =
  {
    fun_tast_hashes = SMap.map Tast.hash_def_list fun_tasts;
    class_tast_hashes = SMap.map Tast.hash_def class_tasts;
    typedef_tast_hashes = SMap.map Tast.hash_def typedef_tasts;
    gconst_tast_hashes = SMap.map Tast.hash_def gconst_tasts;
    module_tast_hashes = SMap.map Tast.hash_def module_tasts;
  }

let empty = Relative_path.Map.empty

let union m1 m2 = Relative_path.Map.union m1 m2

let add m ~key ~data =
  match data with
  | None -> m
  | Some data -> Relative_path.Map.add m ~key ~data
