(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open HoverService

let open_multiline = Str.regexp "^/\\*\\(\\*?\\) *"
let close_multiline = Str.regexp " *\\*/$"
let line_prefix = Str.regexp "^// ?"

(** Tidies up a comment.

    # Multiline comment
    When given a multiline comment (`/* */`), this will do the following tidying:
      1. Strip the leading `/*` and trailing `*/`.
      2. Remove leading whitespace equal to the least amount of whitespace
         before any comment lines after the first (since the first line is on
         the same line as the opening `/*`, it will almost always have only a
         single leading space).

         We remove leading whitespace equal to the least amount rather than
         removing all leading whitespace in order to preserve manual indentation
         of text in the doc block.

      3. Remove leading `*` characters to properly handle box-style multiline
         doc blocks. Without this they would be formatted as Markdown lists.

    Known failure cases:
      1. A doc block which is legitimately just a list of items will need at
         least one non-list item which is under-indented compared to the list in
         order to not strip all of the whitespace from the list items. The
         easiest way to do this is to write a little one line summary before the
         list and place it on its own line instead of directly after the `/*`.

    # Single line comment
    When given a list of single line comments (`//`), just strip the leading
    forward slashes and the first space after the second slash.
*)
let tidy comment =
  let is_line_comment = Str.string_match line_prefix comment 0 in
  let comment =
    if is_line_comment
    then comment
    else
      comment
      |> Str.replace_first open_multiline ""
      |> Str.replace_first close_multiline ""
  in
  let lines = String_utils.split_into_lines comment in
  if is_line_comment
  then
    lines
    |> List.map ~f:(Str.replace_first line_prefix "")
    |> String.concat "\n"
  else
    let line_trimmer = match lines with
      | []
      | [_] -> String.trim
      | _hd :: tail ->
        let get_whitespace_count x =
          String_utils.index_not_opt x " "
        in
        let counts = List.filter_map ~f:get_whitespace_count tail in
        let min =
          List.min_elt counts ~cmp:(fun a b -> a - b)
          |> Option.value ~default:0
        in
        let removal = Str.regexp (Printf.sprintf "^%s\\(\\* ?\\)?" (String.make min ' ')) in
        Str.replace_first removal ""
    in
    lines
    |> List.map ~f:line_trimmer
    |> String.concat "\n"

let symbols_at (file, line, char) tcopt =
  let contents = match file with
    | ServerUtils.FileName file_name ->
      let relative_path = Relative_path.(create Root file_name) in
      File_heap.get_contents relative_path
    | ServerUtils.FileContent content -> Some content
  in
  match contents with
  | None -> []
  | Some contents -> ServerIdentifyFunction.go contents line char tcopt

let type_at (file, line, char) tcopt files_info =
  let open Typing_defs in
  let _, tast = ServerIdeUtils.check_file_input tcopt files_info file in
  match ServerInferType.type_at_pos tast line char with
  | Some (_, (_, Tanon _) as infer_type_results1) ->
    (* The Tanon type doesn't include argument or return types, so it's
       displayed as "[fun]". To try to show something a little more useful, we
       call `returned_type_at_pos`. This will give us the function's return type
       (if it is being invoked). *)
    Some (Option.value
      (ServerInferType.returned_type_at_pos tast line char)
      ~default:infer_type_results1)
  | results -> results

let make_hover_info (file, _line, _char) env_and_ty (occurrence, def_opt) =
  let open SymbolOccurrence in
  let open Typing_defs in
  let snippet = match occurrence, env_and_ty with
    | { name; _ }, None -> Utils.strip_ns name
    | occurrence, Some (env, ty) -> Typing_print.full_with_identity env ty occurrence def_opt
  in
  let addendum = [
    (match def_opt with
      | Some def ->
        begin match def.SymbolDefinition.docblock with
        | Some s -> [s]
        | None ->
          let def_file = match ((Pos.filename def.SymbolDefinition.pos), file) with
            | (p, ServerUtils.FileName fn) when p = Relative_path.default ->
              Relative_path.(create Root fn)
            | fn, _ -> fn
          in
          let def_line = (Pos.end_line def.SymbolDefinition.pos) in
          begin match File_heap.get_contents def_file with
          | None -> []
          | Some contents ->
            let env = Full_fidelity_ast.make_env ~include_line_comments:true def_file in
            let results = Full_fidelity_ast.from_text_with_legacy env contents in
            let finder = Docblock_finder.make_docblock_finder (results.Parser_hack.comments) in
            Docblock_finder.find_docblock finder def_line
            |> Option.map ~f:tidy
            |> Option.filter ~f:begin fun cmt ->
              (* Make sure to not add the `// strict` part of `<?hh // strict`
                 if we're looking for something at the start of the file. *)
              not (def_line = 2 && cmt = "strict")
            end
            |> Option.to_list
          end
        end
      | None -> []);
    (match occurrence, env_and_ty with
      | { type_ = Method _; _ }, Some (_, (_, Tfun _))
      | { type_ = Property _; _ }, Some (_, (_, Tfun _))
      | { type_ = ClassConst _; _ }, Some (_, (_, Tfun _)) ->
        [Printf.sprintf "Full name: `%s`" (Utils.strip_ns occurrence.name)]
      | _ -> []);
  ] |> List.concat in
  { snippet; addendum }

let go env (file, line, char) =
  let position = (file, line, char) in
  let ServerEnv.{ tcopt; files_info; _ } = env in
  let identities = symbols_at position tcopt in
  let env_and_ty = type_at position tcopt files_info in
  (* There are legitimate cases where we expect to have no identities returned,
     so just format the type. *)
  match identities with
  | [] ->
    begin match env_and_ty with
    | Some (env, ty) -> [{ snippet = Typing_print.full_strip_ns env ty; addendum = [] }]
    | None -> []
    end
  | identities ->
    identities
    |> List.map ~f:(make_hover_info (file, line, char) env_and_ty)
