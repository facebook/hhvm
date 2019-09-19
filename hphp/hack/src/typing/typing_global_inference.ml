(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

let folder_name = ".global_inference_artifacts/"

let artifacts_path : string ref = ref ""

let init () =
  (*let path = Path.concat path folder_name in
  let path = Path.to_string path in*)
  let path = Filename.concat "/tmp" folder_name in
  Disk.rm_dir_tree path;
  if not @@ Disk.file_exists path then Disk.mkdir path 0o777

let save_subgraphs global_tvenvs =
  let global_tvenvs =
    List.filter ~f:(fun e -> not @@ IMap.is_empty e) global_tvenvs
  in
  if !artifacts_path = "" then
    artifacts_path := Filename.concat "/tmp" folder_name;
  if global_tvenvs = [] then
    ()
  else
    let out_channel =
      Filename.concat
        !artifacts_path
        ("subgraph_" ^ string_of_int (Ident.tmp ()))
      |> Out_channel.create
    in
    Marshal.to_channel out_channel global_tvenvs [];
    Out_channel.close out_channel
