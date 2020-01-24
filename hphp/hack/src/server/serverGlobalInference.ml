(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module Inf = Typing_inference_env
module Syntax = Full_fidelity_editable_positioned_syntax
module Rewriter = Full_fidelity_rewriter.WithSyntax (Syntax)
module PositionedTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
open Syntax

let scuba_table = Scuba.Table.of_name "hh_global_inference"

let is_not_acceptable ~is_return ty =
  let finder =
    object (this)
      inherit [_] Type_visitor.locl_type_visitor

      method! on_tprim acc _ =
        function
        | Aast.Tvoid when not is_return -> true
        | Aast.Tresource
        | Aast.Tnull
        | Aast.Tnoreturn ->
          true
        | _ -> acc

      (* We consider both dynamic and nothing to be non acceptable as
      they are "too narrow" and imprecise *)
      method! on_tdynamic _ _ = true

      method! on_tnonnull _ _ = true

      (* nothing is never acceptable, for now *)
      method! on_tunion _ _ _ = true

      (* mixed, even though we could infer it and add it, might lead to further
      problems and doesn't gives us a lot of information. More conceptually
      adding mixed annotations says "it is fine to add mixed types everywhere"
      which is not really fine. *)
      method! on_toption acc _ ty =
        match Typing_defs.get_node ty with
        | Typing_defs.Tnonnull -> true
        | _ -> this#on_type acc ty
    end
  in
  finder#on_type false ty

let print_ty ?(is_return = false) ty =
  Option.Monad_infix.(
    if is_not_acceptable ~is_return ty then
      None
    else
      CodemodTypePrinter.print ~allow_nothing:is_return ty >>= fun str ->
      Option.some_if (not @@ String.contains str '#') str)

open Typing_env_types
open ServerGlobalInferenceTypes
open Typing_global_inference

(** Whether a type has type variables with errors. *)
let was_type_correctly_solved errors ty =
  let finder =
    object
      inherit [_] Type_visitor.locl_type_visitor

      method! on_tvar _ _ var = StateErrors.has_error errors var
    end
  in
  not @@ finder#on_type false ty

let ( >>= ) error f =
  match error with
  | Ok data -> f data
  | Error error -> Error error

let ( >>| ) error f =
  match error with
  | Ok data -> f data
  | Error error -> RError error

(** Expects the file list to contain two files, the first of which
exists and can be read. *)
let expect_2_files_read_write ~error_message (files : string list) =
  begin
    match files with
    | a :: b :: _ -> Ok (a, b)
    | _ -> Error error_message
  end
  >>= fun (a, b) ->
  if not @@ Disk.file_exists a then
    Error (Printf.sprintf "Error: %s does not exist" a)
  else
    Ok (a, b)

(** Expects the file list to contain one file, which
exists and can be read. *)
let expect_1_file_read ~error_message (files : string list) =
  begin
    match files with
    | a :: _ -> Ok a
    | _ -> Error error_message
  end
  >>= fun input ->
  if not @@ Disk.file_exists input then
    Error (Printf.sprintf "Error: %s does not exist" input)
  else
    Ok input

module Mode_merge = struct
  let execute files =
    expect_2_files_read_write
      ~error_message:
        "was expecting two files: [directory containing subgraphs] [graph output]"
      files
    >>= fun (input, output) ->
    let subgraphs = Disk.readdir input in
    Hh_logger.log "GI: Merging %d files" (Array.length subgraphs);
    let env =
      Typing_env.empty
        GlobalOptions.default
        (Relative_path.from_root "")
        ~droot:None
    in
    let genv = env.genv in
    let progress_i = ref 0 in
    let log_progress fn =
      Hh_logger.log
        "GI: Merging %d/%d: %s"
        (!progress_i + 1)
        (Array.length subgraphs)
        fn;
      progress_i := !progress_i + 1;
      fn
    in
    (* Merging happens here *)
    let (env, errors) =
      Array.foldi
        ~f:(fun _ (env, errors) subgraph_file ->
          Filename.concat input subgraph_file
          |> log_progress
          |> StateSubConstraintGraphs.load
          |> StateConstraintGraph.merge_subgraphs (env, errors))
        ~init:(env, StateErrors.mk_empty ())
        subgraphs
    in
    let env = { env with genv } in
    Hh_logger.log "GI: Saving to %s" output;
    StateConstraintGraph.save output (env, errors);
    Ok ()
end

module Mode_solve = struct
  let execute files =
    expect_2_files_read_write
      ~error_message:"was expecting two files: [graph] [output]"
      files
    >>= fun (input, output) ->
    StateConstraintGraph.load input
    |> StateSolvedGraph.from_constraint_graph
    |> StateSolvedGraph.save output;
    Ok ()
end

module Mode_export_json = struct
  let execute files =
    expect_2_files_read_write
      ~error_message:"was expecting two files: [env] [json env]"
      files
    >>= fun (input, output) ->
    let (env, _errors) = StateConstraintGraph.load input in
    (* Now gonna convert it to json in order to do further
          analysis in python *)
    let out = Out_channel.create output in
    Out_channel.output_string out "{";
    let is_start = ref false in
    let tyvar_to_json env var =
      let type_to_json ty = "\"" ^ Typing_print.full_i env ty ^ "\"" in
      let bounds_to_json bounds =
        "[" ^ (List.map bounds type_to_json |> String.concat ~sep:", ") ^ "]"
      in
      let tyvar_pos = Inf.get_tyvar_pos env.inference_env var in
      let upper_bounds = Inf.get_tyvar_upper_bounds env.inference_env var in
      let lower_bounds = Inf.get_tyvar_lower_bounds env.inference_env var in
      Printf.sprintf
        "{\"start_line\": \"%d\", \"start_column\": \"%d\", \"end_line\": \"%d\", \"end_column\": \"%d\", \"filename\": \"%s\", \"lower_bounds\": %s, \"upper_bounds\": %s}"
        (fst (Pos.line_column tyvar_pos))
        (snd (Pos.line_column tyvar_pos))
        (fst (Pos.end_line_column tyvar_pos))
        (snd (Pos.end_line_column tyvar_pos))
        (Pos.filename (Pos.to_absolute tyvar_pos))
        (bounds_to_json (ITySet.elements lower_bounds))
        (bounds_to_json (ITySet.elements upper_bounds))
    in
    List.iter
      ~f:(fun var ->
        let key =
          ( if !is_start then
            ","
          else
            "" )
          ^ Printf.sprintf "\n\"#%d\": %s" var (tyvar_to_json env var)
        in
        is_start := true;
        Out_channel.output_string out key)
      (Inf.get_vars env.inference_env);
    Out_channel.output_string out "}";
    Out_channel.close out;
    Ok ()
end

type status_type =
  | SError
  | SNonacceptable
  | SCorrect

type syntax_type =
  | RetType
  | ParamType

type loggable = {
  file: string;
  pos: Pos.t;
  ty: string;
  status: status_type;
  syntax_type: syntax_type;
}

let classify_ty ?(is_return = false) ~syntax_type ~pos ~file errors env ty =
  let has_error = ref false in
  let var_hook var =
    has_error := !has_error || StateErrors.has_error errors var
  in
  let ty = Tast_expand.expand_ty ~var_hook env ty in
  let log =
    {
      file = Relative_path.to_absolute file;
      pos;
      ty = Typing_print.full (Tast_env.tast_env_as_typing_env env) ty;
      status = SCorrect;
      syntax_type;
    }
  in
  if !has_error then
    ({ log with status = SError }, None)
  else
    match print_ty ~is_return ty with
    | Some type_str -> (log, Some type_str)
    | None -> ({ log with status = SNonacceptable }, None)

let log_ty log =
  let status_str =
    match log.status with
    | SError -> "error"
    | SNonacceptable -> "nonacceptable"
    | SCorrect -> "correct"
  in
  EventLogger.log_if_initialized @@ fun () ->
  Scuba.new_sample (Some scuba_table)
  |> Scuba.add_normal "file" log.file
  |> Scuba.add_int "start_line" (fst @@ Pos.line_column log.pos)
  |> Scuba.add_int "start_column" (snd @@ Pos.line_column log.pos)
  |> Scuba.add_int "end_line" (fst @@ Pos.end_line_column log.pos)
  |> Scuba.add_int "end_column" (snd @@ Pos.end_line_column log.pos)
  |> Scuba.add_int
       "is_error"
       ( if log.status = SError then
         1
       else
         0 )
  |> Scuba.add_int
       "is_nonacceptable"
       ( if log.status = SNonacceptable then
         1
       else
         0 )
  |> Scuba.add_int
       "is_ret"
       ( if log.syntax_type = RetType then
         1
       else
         0 )
  |> Scuba.add_int
       "is_param"
       ( if log.syntax_type = ParamType then
         1
       else
         0 )
  |> Scuba.add_normal "ty" log.ty
  |> EventLogger.log;
  Hh_logger.log
    "[%s] %s, type %s"
    status_str
    (Pos.string (Pos.to_absolute log.pos))
    log.ty

let get_first_suggested_type_as_string ~syntax_type errors file type_map node =
  Option.Monad_infix.(
    position file node >>= fun pos ->
    Tast_type_collector.get_from_pos_map (Pos.to_absolute pos) type_map
    >>= fun tys ->
    List.find_map tys ~f:(fun (env, phase_ty) ->
        match phase_ty with
        | Typing_defs.LoclTy ty ->
          let (log, type_str_opt) =
            classify_ty
              ~is_return:(syntax_type = RetType)
              ~syntax_type
              ~pos
              ~file
              errors
              env
              ty
          in
          log_ty log;
          type_str_opt
        | Typing_defs.DeclTy _ -> None))

let get_patches
    errors (type_map : (Tast_env.env * phase_ty) list Pos.AbsolutePosMap.t) file
    =
  let file = Relative_path.create_detect_prefix file in
  if Relative_path.prefix file = Relative_path.Hhi then
    []
  else
    let source_text = Full_fidelity_source_text.from_file file in
    let positioned_tree = PositionedTree.make source_text in
    let root =
      Full_fidelity_editable_positioned_syntax.from_positioned_syntax
        (PositionedTree.root positioned_tree)
    in
    let get_patches node =
      Option.Monad_infix.(
        match syntax node with
        | FunctionDeclarationHeader
            { function_type; function_name; function_parameter_list; _ } ->
          begin
            if is_missing function_type then
              let patch =
                get_first_suggested_type_as_string
                  ~syntax_type:RetType
                  errors
                  file
                  type_map
                  function_name
                >>= fun type_str ->
                position_exclusive file function_type >>| fun pos ->
                ServerRefactorTypes.Insert
                  ServerRefactorTypes.
                    {
                      pos = Pos.to_absolute pos;
                      text = Printf.sprintf ": <<__Soft>> %s " type_str;
                    }
              in
              Option.to_list patch
            else
              []
          end
          @
          let lst =
            match function_parameter_list with
            | { syntax = SyntaxList lst; _ } -> lst
            | _ -> []
          in
          List.concat_map lst ~f:(fun n ->
              let opt =
                match syntax n with
                | ListItem
                    {
                      list_item =
                        {
                          syntax = ParameterDeclaration { parameter_type; _ };
                          _;
                        } as list_item;
                      _;
                    }
                  when is_missing parameter_type ->
                  get_first_suggested_type_as_string
                    ~syntax_type:ParamType
                    errors
                    file
                    type_map
                    list_item
                  >>= fun type_str ->
                  position file list_item >>| fun pos ->
                  ServerRefactorTypes.Insert
                    ServerRefactorTypes.
                      {
                        pos = Pos.to_absolute pos;
                        text = "<<__Soft>> " ^ type_str ^ " ";
                      }
                | _ -> None
              in
              Option.to_list opt)
        | _ -> [])
    in
    let (patches, _) =
      Rewriter.aggregating_rewrite_post
        (fun node patches ->
          let a = get_patches node @ patches in
          (a, Rewriter.Result.Keep))
        root
        []
    in
    patches

module Mode_rewrite = struct
  (** Returns a map indexed by filenames. *)
  let build_positions_map :
      Tast_env.env * (Pos.t * Ident.t) list ->
      Tast_env.env * (Tast_env.env * phase_ty) list Pos.AbsolutePosMap.t SMap.t
      =
   fun (env, positions) ->
    let positions =
      List.map ~f:(fun (p, v) -> (Pos.to_absolute p, v)) positions
    in
    List.fold
      ~init:(env, SMap.empty)
      ~f:(fun (env, map) (pos, var) ->
        let previous_pos_map =
          SMap.find_opt (Pos.filename pos) map
          |> Option.value ~default:Pos.AbsolutePosMap.empty
        in
        let ty = Typing_defs.(mk (Reason.none, Tvar var)) in
        let element =
          Pos.AbsolutePosMap.singleton pos [(env, Typing_defs.LoclTy ty)]
        in
        let data =
          Pos.AbsolutePosMap.union
            ~combine:(fun _ a b -> Some (a @ b))
            previous_pos_map
            element
        in
        (env, SMap.add (Pos.filename pos) data map))
      positions

  let execute files =
    expect_1_file_read
      ~error_message:"was expecting one file: [solvedenv]"
      files
    >>= fun input ->
    let (env, errors, positions) = StateSolvedGraph.load input in
    let (_, positions_map) =
      build_positions_map (Tast_env.typing_env_as_tast_env env, positions)
    in
    let positions_map =
      SMap.filter
        (fun filename _ -> not @@ Int.equal (String.length filename) 0)
        positions_map
    in
    let patches =
      SMap.fold
        (fun filename type_map patches ->
          let new_patches = get_patches errors type_map filename in
          new_patches @ patches)
        positions_map
        []
    in
    Ok patches
end

(* Entry Point *)
let execute mode files =
  match mode with
  | MMerge -> Mode_merge.execute files >>| fun x -> RMerge x
  | MSolve -> Mode_solve.execute files >>| fun x -> RSolve x
  | MExport -> Mode_export_json.execute files >>| fun x -> RExport x
  | MRewrite -> Mode_rewrite.execute files >>| fun x -> RRewrite x
