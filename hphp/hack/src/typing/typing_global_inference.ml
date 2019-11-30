(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Typing_env_types

module StateErrors = struct
  module IdentMap = WrappedMap.Make (Ident)

  type t = Errors.error list IdentMap.t ref

  let mk_empty () = ref IdentMap.empty

  let get_errors t id = Option.value (IdentMap.find_opt id !t) ~default:[]

  let add t id err = t := IdentMap.add id (err :: get_errors t id) !t

  let has_error t id = not @@ List.is_empty @@ get_errors t id

  let elements t = IdentMap.elements !t
end

let make_error_callback errors var ?code msgl =
  let code =
    Option.value code ~default:Error_codes.Typing.(err_code UnifyError)
  in
  StateErrors.add errors var (Errors.make_error code msgl)

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

let artifacts_path : string ref = ref ""

module StateConstraintGraph = struct
  include StateFunctor (struct
    type t = env * StateErrors.t
  end)

  let merge_subgraph (env, errors) (_pos, subgraph) =
    let env =
      IMap.fold
        (fun var tyvar_info env ->
          let current_tyvar_info : tyvar_info_ =
            Typing_env.get_tyvar_info env var
          in
          let pos =
            match (tyvar_info.tyvar_pos, current_tyvar_info.tyvar_pos) with
            | (p, t) when Pos.equal t Pos.none -> p
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
          |> ITySet.fold
               (fun bound env ->
                 Typing_subtype.sub_type_i
                   env
                   (Typing_defs.LoclType
                      (Typing_reason.Rwitness pos, Typing_defs.Tvar var))
                   bound
                   (make_error_callback errors var))
               tyvar_info.upper_bounds
          |> ITySet.fold
               (fun bound env ->
                 Typing_subtype.sub_type_i
                   env
                   bound
                   (Typing_defs.LoclType
                      (Typing_reason.Rwitness pos, Typing_defs.Tvar var))
                   (make_error_callback errors var))
               tyvar_info.lower_bounds)
        subgraph
        env
    in
    (env, errors)

  let merge_subgraphs (env, errors) subgraphs =
    List.fold ~f:merge_subgraph ~init:(env, errors) subgraphs
end

module StateSubConstraintGraphs = struct
  include StateFunctor (struct
    type t = global_tvenv_with_pos list
  end)

  let save subconstraints =
    let subconstraints =
      List.filter ~f:(fun (_, e) -> not @@ IMap.is_empty e) subconstraints
    in
    if List.is_empty subconstraints then
      ()
    else
      let path =
        Filename.concat
          !artifacts_path
          ("subgraph_" ^ string_of_int (Ident.tmp ()))
      in
      save path subconstraints
end

module StateSolvedGraph = struct
  include StateFunctor (struct
    type t = env * StateErrors.t * (Pos.t * int) list
  end)

  let save path t = save path t

  let from_constraint_graph (constraintgraph, errors) =
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
    Typing_solver.use_bind_to_equal_bound := false;

    (* For any errors seen during the last step (that is graph merging), we bind
    the corresponding tyvar to Terr *)
    let env =
      IMap.fold
        (fun var _ env ->
          if StateErrors.has_error errors var then
            Typing_solver.bind env var (Typing_reason.Rnone, Typing_defs.Terr)
          else
            env)
        env.tvenv
        env
    in
    let env =
      Typing_solver.close_tyvars_and_solve_gi env (make_error_callback errors)
    in
    let env =
      Typing_solver.solve_all_unsolved_tyvars_gi
        env
        (make_error_callback errors)
    in
    Typing_solver.use_bind_to_equal_bound := true;
    (env, errors, positions)
end

let set_path () =
  let tmp = Tmp.temp_dir GlobalConfig.tmp_dir "gi_artifacts" in
  artifacts_path := tmp

let get_path () = !artifacts_path

let restore_path s = artifacts_path := s

let init () =
  let path = !artifacts_path in
  Hh_logger.log "Global artifacts path: %s" path;
  Disk.rm_dir_tree path;
  if not @@ Disk.file_exists path then Disk.mkdir path 0o777
