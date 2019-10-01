(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Typing_env_types

module type MarshalledData = sig
  type t
end

module StateFunctor (M : MarshalledData) = struct
  type t = M.t

  let load path =
    let channel = In_channel.create path in
    let data : t = Marshal.from_channel channel in
    In_channel.close channel;
    data

  let save path t =
    let out_channel = Out_channel.create path in
    Marshal.to_channel out_channel t [];
    Out_channel.close out_channel
end

let folder_name = ".global_inference_artifacts/"

let artifacts_path : string ref = ref ""

module StateConstraintGraph = struct
  include StateFunctor (struct
    type t = env
  end)

  let merge_subgraph env subgraph =
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

  let merge_subgraphs env subgraphs =
    List.fold ~f:merge_subgraph ~init:env subgraphs
end

module StateSubConstraintGraphs = struct
  include StateFunctor (struct
    type t = global_tvenv list
  end)

  let save subcontraints =
    let subcontraints =
      List.filter ~f:(fun e -> not @@ IMap.is_empty e) subcontraints
    in
    if !artifacts_path = "" then
      artifacts_path := Filename.concat "/tmp" folder_name;
    if subcontraints = [] then
      ()
    else
      let path =
        Filename.concat
          !artifacts_path
          ("subgraph_" ^ string_of_int (Ident.tmp ()))
      in
      save path subcontraints
end

module StateSolvedGraph = struct
  include StateFunctor (struct
    type t = env * (Pos.t * int) list
  end)

  let from_constraint_graph constraintgraph =
    let extract_pos = function
      | LocalTyvar tyvar -> tyvar.tyvar_pos
      | GlobalTyvar -> Pos.none
    in
    let positions =
      IMap.fold
        (fun var tyvar_info l -> (extract_pos tyvar_info, var) :: l)
        constraintgraph.tvenv
        []
    in
    let (_, tyvars) = List.unzip positions in
    let env = { constraintgraph with tyvars_stack = [(Pos.none, tyvars)] } in
    (Typing_solver.close_tyvars_and_solve env Errors.unify_error, positions)
end

let init () =
  let path = Filename.concat "/tmp" folder_name in
  Disk.rm_dir_tree path;
  if not @@ Disk.file_exists path then Disk.mkdir path 0o777
