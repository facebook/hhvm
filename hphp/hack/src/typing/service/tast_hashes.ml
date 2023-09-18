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
    fun_tast_hashes = SMap.map Tast.hash_def_with_dynamic fun_tasts;
    class_tast_hashes = SMap.map Tast.hash_def_with_dynamic class_tasts;
    typedef_tast_hashes = SMap.map Tast.hash_def typedef_tasts;
    gconst_tast_hashes = SMap.map Tast.hash_def gconst_tasts;
    module_tast_hashes = SMap.map Tast.hash_def module_tasts;
  }

let union_by_names x y =
  {
    fun_tast_hashes = SMap.union x.fun_tast_hashes y.fun_tast_hashes;
    class_tast_hashes = SMap.union x.class_tast_hashes y.class_tast_hashes;
    typedef_tast_hashes = SMap.union x.typedef_tast_hashes y.typedef_tast_hashes;
    gconst_tast_hashes = SMap.union x.gconst_tast_hashes y.gconst_tast_hashes;
    module_tast_hashes = SMap.union x.module_tast_hashes y.module_tast_hashes;
  }

let hash_tasts_by_file : Tast.by_names Relative_path.Map.t -> t =
  Relative_path.Map.map ~f:hash_tasts

let error_while_hashing
    { Tast.fun_tasts; class_tasts; typedef_tasts; gconst_tasts; module_tasts } :
    by_names =
  let minus_one _ = -1 in
  {
    fun_tast_hashes = SMap.map minus_one fun_tasts;
    class_tast_hashes = SMap.map minus_one class_tasts;
    typedef_tast_hashes = SMap.map minus_one typedef_tasts;
    gconst_tast_hashes = SMap.map minus_one gconst_tasts;
    module_tast_hashes = SMap.map minus_one module_tasts;
  }

let empty = Relative_path.Map.empty

let add m ~key ~data =
  match data with
  | None -> m
  | Some data -> Relative_path.Map.add m ~key ~data

let is_enabled tcopt = TypecheckerOptions.dump_tast_hashes tcopt

let map ctx path tasts =
  let data =
    Timeout.with_timeout
      ~timeout:30
      ~on_timeout:(fun _timings -> error_while_hashing tasts)
      ~do_:(fun _timeout ->
        let tasts = Tast.map_by_names tasts ~f:(Tast_expand.expand_def ctx) in
        hash_tasts tasts)
  in
  add empty ~key:path ~data:(Some data)

let reduce xs ys =
  Relative_path.Map.union
    ~combine:(fun _key x y -> Some (union_by_names x y))
    xs
    ys

let finalize ~progress ~init_id ~recheck_id tast_hashes =
  progress "Converting TAST hashes to JSON";
  let tast_hashes_json = yojson_of_t tast_hashes in
  progress "Writing TAST hashes to disk";
  let tast_dir =
    Tmp.make_dir_in_tmp ~description_what_for:"tast_hashes" ~root:None
  in
  let tast_hashes_file =
    Filename.concat
      tast_dir
      (Printf.sprintf
         "initId%s_recheckId%s.json"
         init_id
         (Option.value recheck_id ~default:"None"))
  in
  Out_channel.with_file tast_hashes_file ~f:(fun out ->
      Yojson.Safe.pretty_to_channel out tast_hashes_json)
