(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Typing_env_types

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

let merge_subgraph_in_env subgraph env =
  IMap.fold
    (fun var tyvar_info env ->
      let current_tyvar_info : tyvar_info_ =
        Typing_env.get_tyvar_info env var
      in
      let pos =
        match (tyvar_info.tyvar_pos, current_tyvar_info.tyvar_pos) with
        | (p, t) when t = Pos.none -> p
        | (_, p) -> p
      in
      let current_tyvar_info =
        {
          current_tyvar_info with
          appears_covariantly =
            tyvar_info.appears_covariantly
            || current_tyvar_info.appears_covariantly;
          appears_contravariantly =
            tyvar_info.appears_contravariantly
            || current_tyvar_info.appears_contravariantly;
          tyvar_pos = pos;
        }
      in
      (* We store in the env the updated tyvar_info_ *)
      Typing_env.update_tyvar_info env var current_tyvar_info
      (* Add the missing upper and lower bounds - and do the transitive closure *)
      |> TySet.fold
           (fun bound env ->
             Typing_subtype.add_tyvar_upper_bound_and_close
               env
               var
               bound
               Errors.unify_error)
           tyvar_info.upper_bounds
      |> TySet.fold
           (fun bound env ->
             Typing_subtype.add_tyvar_lower_bound_and_close
               env
               var
               bound
               Errors.unify_error)
           tyvar_info.lower_bounds)
    subgraph
    env
