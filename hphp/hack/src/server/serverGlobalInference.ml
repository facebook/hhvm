(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_env_types
open ServerGlobalInferenceTypes

let expect_2_files ~error_message (files : string list) =
  match files with
  | a :: b :: _ -> Ok (a, b)
  | _ -> Error error_message

let expect_1_file ~error_message (files : string list) =
  match files with
  | a :: _ -> Ok a
  | _ -> Error error_message

let ( >>= ) error f =
  match error with
  | Ok data -> f data
  | Error error -> Error error

let ( >>| ) error f =
  match error with
  | Ok data -> f data
  | Error error -> RError error

module Mode_merge = struct
  let execute files =
    expect_2_files
      ~error_message:
        "was expecting two files: [directory containing subgraphs] [graph output]"
      files
    >>= fun (input, output) ->
    if not @@ Disk.file_exists input then
      Error (Printf.sprintf "Error: %s does not exists" input)
    else
      Ok ()
      >>= fun () ->
      let subgraphs = Disk.readdir input in
      let env =
        Typing_env.empty
          GlobalOptions.default
          (Relative_path.from_root "")
          ~droot:None
      in
      let genv = env.genv in
      let subtype_prop = env.subtype_prop in
      let env =
        Array.foldi
          ~f:(fun _ env subgraph_file ->
            let channel =
              Filename.concat input subgraph_file |> In_channel.create
            in
            let subgraphs : global_tvenv list = Marshal.from_channel channel in
            In_channel.close channel;
            List.fold
              ~f:(fun env subgraph ->
                Typing_global_inference.merge_subgraph_in_env subgraph env)
              ~init:env
              subgraphs)
          ~init:env
          subgraphs
      in
      (* Really needed to restore subtype prop? *)
      let env = { env with genv; subtype_prop } in
      let out_channel = Out_channel.create output in
      Marshal.to_channel out_channel env [];
      Out_channel.close out_channel;
      Ok ()
end

module Mode_solve = struct
  let execute files =
    expect_2_files
      ~error_message:"was expecting two files: [graph] [output]"
      files
    >>= fun (input, output) ->
    if not @@ Disk.file_exists input then
      Error (Printf.sprintf "Error: %s does not exist" input)
    else
      Ok ()
      >>= fun () ->
      let channel = In_channel.create input in
      let env : env = Marshal.from_channel channel in
      In_channel.close channel;
      let env =
        {
          env with
          tyvars_stack =
            [(Pos.none, IMap.fold (fun tyvar _ l -> tyvar :: l) env.tvenv [])];
        }
      in
      let env = Typing_solver.close_tyvars_and_solve env Errors.unify_error in
      let out_channel = Out_channel.create output in
      Marshal.to_channel out_channel env [];
      Out_channel.close out_channel;
      Ok ()
end

module Mode_export_json = struct
  let execute files =
    expect_2_files
      ~error_message:"was expecting two files: [env] [jsonenv]"
      files
    >>= fun (input, output) ->
    if not @@ Disk.file_exists input then
      Error (Printf.sprintf "Error: %s does not exists" input)
    else
      Ok ()
      >>= fun () ->
      let channel = In_channel.create input in
      let env : env = Marshal.from_channel channel in
      In_channel.close channel;

      (* Now gonna convert it to json in order to do further
      analysis in python *)
      let out = Out_channel.create output in
      Out_channel.output_string out "{";
      let is_start = ref false in
      let tyvar_to_json env = function
        | LocalTyvar tyvar ->
          let type_to_json ty = "\"" ^ Typing_print.full env ty ^ "\"" in
          let bounds_to_json bounds =
            if List.length bounds = 0 then
              "[]"
            else
              "["
              ^ List.fold
                  ~init:(type_to_json @@ List.hd_exn bounds)
                  ~f:(fun acc e -> acc ^ ", " ^ type_to_json e)
                  (List.tl_exn bounds)
              ^ "]"
          in
          Printf.sprintf
            "{\"start_line\": \"%d\", \"start_column\": \"%d\", \"end_line\": \"%d\", \"end_column\": \"%d\", \"filename\": \"%s\", \"lower_bounds\": %s, \"upper_bounds\": %s}"
            (fst (Pos.line_column tyvar.tyvar_pos))
            (snd (Pos.line_column tyvar.tyvar_pos))
            (fst (Pos.end_line_column tyvar.tyvar_pos))
            (snd (Pos.end_line_column tyvar.tyvar_pos))
            (Pos.filename (Pos.to_absolute tyvar.tyvar_pos))
            (bounds_to_json (TySet.elements tyvar.lower_bounds))
            (bounds_to_json (TySet.elements tyvar.upper_bounds))
        | GlobalTyvar -> "global"
      in
      IMap.iter
        (fun var tyvar ->
          let key =
            ( if !is_start then
              ","
            else
              "" )
            ^ Printf.sprintf "\n\"#%d\": %s" var (tyvar_to_json env tyvar)
          in
          is_start := true;
          Out_channel.output_string out key)
        env.tvenv;
      Out_channel.output_string out "}";
      Out_channel.close out;
      Ok ()
end

(* Entry Point *)
let execute mode files =
  match mode with
  | MMerge -> Mode_merge.execute files >>| (fun x -> RMerge x)
  | MSolve -> Mode_solve.execute files >>| (fun x -> RSolve x)
  | MExport -> Mode_export_json.execute files >>| (fun x -> RExport x)
