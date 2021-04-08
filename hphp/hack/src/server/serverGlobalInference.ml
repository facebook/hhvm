(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Inf = Typing_inference_env
module Syntax = Full_fidelity_editable_positioned_syntax
module Rewriter = Full_fidelity_rewriter.WithSyntax (Syntax)
module PositionedTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module Reason = Typing_reason
module ITySet = Internal_type_set
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
  let execute ctx files =
    expect_2_files_read_write
      ~error_message:
        "was expecting two files: [directory containing subgraphs] [graph output]"
      files
    >>= fun (input, output) ->
    let subgraphs = Disk.readdir input in
    Hh_logger.log "GI: Merging %d files" (Array.length subgraphs);
    let subgraphs =
      List.map (Array.to_list subgraphs) ~f:(Filename.concat input)
    in
    let subgraphs = List.map subgraphs ~f:StateSubConstraintGraphs.load in
    let merged_graph = StateConstraintGraph.merge_subgraphs ctx subgraphs in
    Hh_logger.log "GI: Saving to %s" output;
    StateConstraintGraph.save output merged_graph;
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
    let (_type_map, env, _errors) = StateConstraintGraph.load input in
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
  | PropType

type loggable = {
  file: string;
  pos: Pos.t;
  ty: string;
  status: status_type;
  syntax_type: syntax_type;
}

let remove_namespace_prefix type_str =
  let remove_all_occurrences needle haystack =
    let substring_index = String_utils.substring_index needle in
    let rec f haystack =
      let start_index = substring_index haystack in
      if start_index > -1 then
        let prefix = String_utils.string_before haystack start_index in
        let suffix =
          String_utils.string_after haystack (start_index + String.length needle)
        in
        f (prefix ^ suffix)
      else
        haystack
    in
    f haystack
  in
  type_str
  |> remove_all_occurrences "HH\\Lib\\"
  |> remove_all_occurrences "HH\\"

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
    | Some type_str -> (log, Some (remove_namespace_prefix type_str))
    | None -> ({ log with status = SNonacceptable }, None)

let log_ty log =
  EventLogger.log_if_initialized @@ fun () ->
  Scuba.new_sample (Some scuba_table)
  |> Scuba.add_normal "file" log.file
  |> Scuba.add_int "start_line" (fst @@ Pos.line_column log.pos)
  |> Scuba.add_int "start_column" (snd @@ Pos.line_column log.pos)
  |> Scuba.add_int "end_line" (fst @@ Pos.end_line_column log.pos)
  |> Scuba.add_int "end_column" (snd @@ Pos.end_line_column log.pos)
  |> Scuba.add_int
       "is_error"
       (match log.status with
       | SError -> 1
       | _ -> 0)
  |> Scuba.add_int
       "is_nonacceptable"
       (match log.status with
       | SNonacceptable -> 1
       | _ -> 0)
  |> Scuba.add_int
       "is_ret"
       (match log.syntax_type with
       | RetType -> 1
       | _ -> 0)
  |> Scuba.add_int
       "is_param"
       (match log.syntax_type with
       | ParamType -> 1
       | _ -> 0)
  |> Scuba.add_normal "ty" log.ty
  |> EventLogger.log

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
              ~is_return:
                (match syntax_type with
                | RetType -> true
                | _ -> false)
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
    ?(files_contents : string Relative_path.Map.t option)
    errors
    (type_map : (Tast_env.env * phase_ty) list Pos.AbsolutePosMap.t)
    file =
  let file = Relative_path.create_detect_prefix file in
  if Relative_path.(is_hhi (prefix file)) then
    []
  else
    let source_text =
      match files_contents with
      | Some files_contents ->
        let contents =
          match Relative_path.Map.find_opt files_contents file with
          | Some contents -> contents
          | None ->
            failwith
            @@ Printf.sprintf
                 "patches: no file contents for %s"
                 (Relative_path.suffix file)
        in
        Full_fidelity_source_text.make file contents
      | None -> Full_fidelity_source_text.from_file file
    in
    let positioned_tree = PositionedTree.make source_text in
    let root =
      Full_fidelity_editable_positioned_syntax.from_positioned_syntax
        (PositionedTree.root positioned_tree)
    in
    let get_patches node =
      (* In the code below, you'll see a re-occurring pattern: the use of
       * Option.is_some and two calls to get_first_suggested_type_as_string.
       *
       * We support two types of positions in the position-to-type map:
       *
       * 1. When the return type is missing, the position encoded in the
       *    reason is that of the function symbol.
       *
       * 2. When the return type is not missing, the position encoded in
       *    the reason might be that of the original return type).
       *)
      Option.Monad_infix.(
        match syntax node with
        | FunctionDeclarationHeader
            { function_type; function_name; function_parameter_list; _ } ->
          begin
            let patch =
              Option.first_some
                (get_first_suggested_type_as_string
                   ~syntax_type:RetType
                   errors
                   file
                   type_map
                   function_name)
                (get_first_suggested_type_as_string
                   ~syntax_type:RetType
                   errors
                   file
                   type_map
                   function_type)
              >>= fun type_str ->
              position_exclusive file function_type >>| fun pos ->
              if is_missing function_type then
                ServerRefactorTypes.Insert
                  ServerRefactorTypes.
                    {
                      pos = Pos.to_absolute pos;
                      text = Printf.sprintf ": <<__Soft>> %s " type_str;
                    }
              else
                ServerRefactorTypes.Replace
                  ServerRefactorTypes.
                    {
                      pos = Pos.to_absolute pos;
                      text = Printf.sprintf "<<__Soft>> %s" type_str;
                    }
            in
            Option.to_list patch
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
                          syntax =
                            ParameterDeclaration
                              {
                                parameter_attribute;
                                parameter_visibility;
                                parameter_call_convention;
                                parameter_readonly = _;
                                parameter_type;
                                parameter_name;
                                parameter_default_value = _;
                              };
                          _;
                        };
                      _;
                    } ->
                  Option.first_some
                    (get_first_suggested_type_as_string
                       ~syntax_type:ParamType
                       errors
                       file
                       type_map
                       parameter_name)
                    (get_first_suggested_type_as_string
                       ~syntax_type:ParamType
                       errors
                       file
                       type_map
                       parameter_type)
                  >>= fun type_str ->
                  if is_missing parameter_type then
                    (* if there is nothing in front of the parameter name,
                     * we will use the paramter name's position, instead
                     * of the parameter type's position. Because, in this
                     * case, if the parameter name is the first thing on
                     * a line, we will get Pos 1:1 for the parameter's
                     * position, regardless of leading whitespace *)
                    let pos =
                      if
                        List.for_all
                          [
                            parameter_attribute;
                            parameter_visibility;
                            parameter_call_convention;
                          ]
                          ~f:is_missing
                      then
                        position_exclusive file parameter_name
                      else
                        position_exclusive file parameter_type
                    in
                    pos >>| fun pos ->
                    ServerRefactorTypes.Insert
                      ServerRefactorTypes.
                        {
                          pos = Pos.to_absolute pos;
                          text = "<<__Soft>> " ^ type_str ^ " ";
                        }
                  else
                    position_exclusive file parameter_type >>| fun pos ->
                    ServerRefactorTypes.Replace
                      ServerRefactorTypes.
                        {
                          pos = Pos.to_absolute pos;
                          text = "<<__Soft>> " ^ type_str;
                        }
                | _ -> None
              in
              Option.to_list opt)
        | PropertyDeclaration
            {
              property_attribute_spec;
              property_modifiers;
              property_type;
              property_declarators;
              _;
            } ->
          Option.value ~default:[]
          @@
          let declarator_and_has_multiple =
            match syntax property_declarators with
            | SyntaxList (li :: tl) ->
              (match syntax li with
              | ListItem { list_item = declarator; _ } ->
                Some (declarator, not @@ List.is_empty tl)
              | _ -> failwith "expected a ListItem")
            | _ -> failwith "expected a non-empty SyntaxList"
          in
          let modifier =
            match syntax property_modifiers with
            | SyntaxList (first_modifier :: _) -> Some first_modifier
            | _ -> failwith "expected at least 1 property modifier"
          in
          modifier >>= fun modifier ->
          declarator_and_has_multiple >>= fun (declarator, has_multiple) ->
          ( if has_multiple then begin
            Hh_logger.log
              "%s"
              ( "warning: generate patch: can't generate patch for"
              ^ " class property with multiple declarators." );
            None
          end else
            Some () )
          >>= fun () ->
          Option.first_some
            (match syntax declarator with
            | PropertyDeclarator { property_name; _ } ->
              get_first_suggested_type_as_string
                ~syntax_type:PropType
                errors
                file
                type_map
                property_name
            | _ -> failwith "expected a PropertyDeclarator")
            (get_first_suggested_type_as_string
               ~syntax_type:PropType
               errors
               file
               type_map
               property_type)
          >>= fun type_str ->
          ( if is_missing property_attribute_spec then
            position file modifier >>| fun pos ->
            [
              ServerRefactorTypes.(
                Insert { pos = Pos.to_absolute pos; text = "<<__Soft>> " });
            ]
          else (
            Hh_logger.log
              "%s"
              ( "warning: generate patch: can't generate patch for"
              ^ " class property that already has property attributes." );
            None
          ) )
          >>= fun soft_patch ->
          position_exclusive file property_type >>| fun pos ->
          let patch =
            if is_missing property_type then
              ServerRefactorTypes.(
                Insert { pos = Pos.to_absolute pos; text = type_str ^ " " })
            else
              ServerRefactorTypes.(
                Replace { pos = Pos.to_absolute pos; text = type_str })
          in
          patch :: soft_patch
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
  (** Returns a map of types indexed by filenames and positions *)
  let build_positions_map :
      Tast_env.env ->
      global_type_map ->
      (Tast_env.env * phase_ty) list Pos.AbsolutePosMap.t SMap.t =
   fun env pos_to_type_map ->
    (* find the types in the pos -> ty map, and convert to a
     * filename -> pos -> [(env, ty)] map *)
    Pos.AbsolutePosMap.fold
      (fun pos ty ->
        let ty = LoclTy ty in
        SMap.update (Pos.filename pos) @@ function
        | None -> Some (Pos.AbsolutePosMap.singleton pos [(env, ty)])
        | Some m -> Some (Pos.AbsolutePosMap.add pos [(env, ty)] m))
      pos_to_type_map
      SMap.empty

  let get_patches
      ?(files_contents : string Relative_path.Map.t option)
      (graph : StateSolvedGraph.t) : ServerRefactorTypes.patch list =
    let (env, errors, type_map) = graph in
    let positions_map =
      build_positions_map (Tast_env.typing_env_as_tast_env env) type_map
    in
    let positions_map =
      SMap.filter
        (fun filename _ -> not @@ Int.equal (String.length filename) 0)
        positions_map
    in
    let patches =
      SMap.fold
        (fun filename type_map patches ->
          let new_patches =
            get_patches ?files_contents errors type_map filename
          in
          new_patches @ patches)
        positions_map
        []
    in
    patches

  let execute files =
    expect_1_file_read
      ~error_message:"was expecting one file: [solvedenv]"
      files
    >>= fun input -> Ok (get_patches (StateSolvedGraph.load input))
end

(* Entry Point *)
let execute ctx mode files =
  match mode with
  | MMerge -> Mode_merge.execute ctx files >>| fun x -> RMerge x
  | MSolve -> Mode_solve.execute files >>| fun x -> RSolve x
  | MExport -> Mode_export_json.execute files >>| fun x -> RExport x
  | MRewrite -> Mode_rewrite.execute files >>| fun x -> RRewrite x
