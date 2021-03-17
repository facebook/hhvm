(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv
module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_editable_syntax
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax (Syntax)
module PositionedSyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module Rewriter = Full_fidelity_rewriter.WithSyntax (Syntax)
module Token = Full_fidelity_editable_token
module TokenKind = Full_fidelity_token_kind

exception CircularDependency of string

(* remove <?hh if present *)
let without_markup_suffix node =
  Rewriter.rewrite_pre_and_stop
    (fun inner ->
      match Syntax.syntax inner with
      | Syntax.MarkupSuffix _ ->
        Rewriter.Replace (Syntax.make_missing SourceText.empty 0)
      | _ -> Rewriter.Keep)
    node

(* replace `namespace Foo;` with `namespace Foo { ... }` *)
let normalize_namespace_body node =
  match Syntax.syntax node with
  | Syntax.Script s ->
    begin
      match Syntax.syntax s.script_declarations with
      | Syntax.SyntaxList declarations ->
        begin
          match
            List.find_mapi declarations (fun i f ->
                match Syntax.syntax f with
                | Syntax.NamespaceDeclaration ns ->
                  begin
                    match Syntax.syntax ns.namespace_body with
                    | Syntax.NamespaceEmptyBody _ ->
                      let inner =
                        List.drop declarations (i + 1)
                        |> Syntax.make_list SourceText.empty 0
                      in
                      let open_brace =
                        Token.create TokenKind.LeftBrace "{" [] []
                        |> Syntax.make_token
                      in
                      let close_brace =
                        Token.create TokenKind.RightBrace "}" [] []
                        |> Syntax.make_token
                      in
                      let body =
                        Syntax.make_namespace_body open_brace inner close_brace
                      in
                      let ns =
                        Syntax.make_namespace_declaration
                          ns.namespace_header
                          body
                      in
                      let pre = List.take declarations i in
                      Some
                        (Syntax.make_script
                           (Syntax.make_list SourceText.empty 0 (pre @ [ns])))
                    | _ -> Some node
                  end
                | _ -> None)
          with
          | Some replacement -> replacement
          | None ->
            (* no namespace statement; add a namespace { ... }: if there are
             * any namespace declarations in the concatenated file, all
             * statements must be in namespace blocks *)
            let open_brace =
              Token.create TokenKind.LeftBrace "{" [] [] |> Syntax.make_token
            in
            let close_brace =
              Token.create TokenKind.RightBrace "}" [] [] |> Syntax.make_token
            in
            let ns =
              Syntax.make_namespace_declaration
                (Syntax.make_namespace_declaration_header
                   (Syntax.make_token
                      (Token.create TokenKind.Namespace "namespace" [] []))
                   (Syntax.make_missing SourceText.empty 0))
                (Syntax.make_namespace_body
                   open_brace
                   (Syntax.make_list SourceText.empty 0 declarations)
                   close_brace)
            in
            Syntax.make_script (Syntax.make_list SourceText.empty 0 [ns])
        end
      | _ -> node
    end
  | _ -> node

(* Apply any necessary AST transformations, then return the source code *)
let get_normalized_content (path : Relative_path.t) =
  let source_text = SourceText.from_file path in
  let mode = Full_fidelity_parser.parse_mode source_text in
  let env = Full_fidelity_parser_env.make ?mode () in
  let tree =
    PositionedSyntaxTree.make ~env source_text
    |> SyntaxTransforms.editable_from_positioned
    |> without_markup_suffix
    |> normalize_namespace_body
  in
  "///// " ^ Relative_path.suffix path ^ " /////\n" ^ Syntax.text tree

(* return all files with the specified prefix as a single file. This will:
 * - resolve inclusion order for class definitions
 * - remove `<?hh` headers
 * - rewrite namespace statements so that concatenation is valid
 *)
let go (genv : ServerEnv.genv) (env : ServerEnv.env) (prefixes : string list) =
  let ctx = Provider_utils.ctx_from_server_env env in
  let deps_mode = Provider_context.get_deps_mode ctx in
  let file_filter (path : string) =
    FindUtils.file_filter path
    && List.exists prefixes (fun prefix ->
           String_utils.string_starts_with path prefix)
  in
  let path_filter (path : Relative_path.t) =
    file_filter (Relative_path.to_absolute path)
  in
  let paths =
    genv.indexer file_filter ()
    |> List.map ~f:Relative_path.create_detect_prefix
  in
  let naming_table = env.ServerEnv.naming_table in
  let dependent_files (path : Relative_path.t) =
    let fileinfo = Naming_table.get_file_info naming_table path in
    match fileinfo with
    | Some FileInfo.{ classes; _ } ->
      let classes =
        let open Typing_deps in
        List.fold_left
          ~init:(DepSet.make deps_mode)
          ~f:(fun acc (_, class_id) ->
            DepSet.add acc (Dep.make (hash_mode deps_mode) (Dep.Type class_id)))
          classes
      in
      let deps =
        Typing_deps.add_extend_deps deps_mode classes
        |> Typing_deps.Files.get_files
        |> Relative_path.Set.filter ~f:path_filter
      in
      Relative_path.Set.remove deps path
    | _ -> Relative_path.Set.empty
  in
  let shallow_dependents =
    List.fold_left
      ~init:Relative_path.Map.empty
      ~f:(fun (acc : Relative_path.Set.t Relative_path.Map.t) path ->
        Relative_path.Map.add ~key:path ~data:(dependent_files path) acc)
      paths
  in
  let rec get_recursive_dependents path =
    match Relative_path.Map.find_opt shallow_dependents path with
    | Some shallow ->
      Relative_path.Set.fold
        ~init:shallow
        ~f:(fun dep_path acc ->
          Relative_path.Set.union acc @@ get_recursive_dependents dep_path)
        shallow
    | _ -> Relative_path.Set.empty
  in
  let recursive_dependents =
    Relative_path.Map.map
      ~f:(fun paths ->
        Relative_path.Set.fold
          ~init:paths
          ~f:(fun path acc ->
            Relative_path.Set.union acc @@ get_recursive_dependents path)
          paths)
      shallow_dependents
  in
  let recursive_dependencies =
    List.map
      ~f:(fun dependent ->
        let dependencies =
          List.filter
            ~f:(fun dependency ->
              match
                Relative_path.Map.find_opt recursive_dependents dependency
              with
              | Some dependents -> Relative_path.Set.mem dependents dependent
              | _ -> false)
            paths
        in
        (dependent, Relative_path.Set.of_list dependencies))
      paths
    |> Relative_path.Map.of_list
  in
  let rec sort (visited : Relative_path.t list) (rest : Relative_path.Set.t) =
    let files_without_deps =
      Relative_path.Set.filter rest (fun path ->
          match Relative_path.Map.find_opt recursive_dependencies path with
          | Some deps ->
            (* any dependencies that aren't in `rest` must have already
             * been visited *)
            let pending_deps = Relative_path.Set.inter deps rest in
            Relative_path.Set.is_empty pending_deps
          | None -> true)
    in
    ( if Relative_path.Set.is_empty files_without_deps then
      (* everything has an unsatisifed dependency, so error out *)
      let visited_pretty =
        List.map ~f:Relative_path.to_absolute visited |> String.concat ~sep:", "
      in
      let rest_pretty =
        List.map
          (Relative_path.Set.elements rest)
          (fun (path : Relative_path.t) ->
            let deps =
              Relative_path.Map.find recursive_dependencies path
              |> Relative_path.Set.inter rest
              |> Relative_path.Set.elements
              |> List.map ~f:Relative_path.to_absolute
              |> String.concat ~sep:", "
            in
            Relative_path.to_absolute path ^ "[" ^ deps ^ "]")
        |> String.concat ~sep:", "
      in
      raise
        (CircularDependency
           ( "circular dependency detected:\nvisited: "
           ^ visited_pretty
           ^ "\nrest: "
           ^ rest_pretty )) );
    let rest = Relative_path.Set.diff rest files_without_deps in
    let visited = visited @ Relative_path.Set.elements files_without_deps in
    if Relative_path.Set.is_empty rest then
      (visited, rest)
    else
      sort visited rest
  in
  let (ordered, _) = sort [] (Relative_path.Set.of_list paths) in
  List.map ~f:get_normalized_content ordered |> String.concat ~sep:"\n"
