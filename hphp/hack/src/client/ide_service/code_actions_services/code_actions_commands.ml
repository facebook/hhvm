(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let max_file_content_lines = 400

let cursor_l = "[DIAGNOSTIC_START]"

let cursor_r = "[DIAGNOSTIC_END]"

let extract_prompt_context buf ~ctxt_pos ~claim_pos =
  let path = Pos.(filename ctxt_pos) in
  let ctxt_start_ln = Pos.line ctxt_pos - 1
  and ctxt_end_ln = Pos.end_line ctxt_pos - 1 in
  let (start_ln, end_ln, start_col, end_col) =
    let (start_ln, end_ln, start_col, end_col) =
      Pos.info_pos_extended claim_pos
    in
    (start_ln - 1, end_ln - 1, start_col - 1, end_col - 1)
  in
  let line_num_width = 1 + (String.length @@ string_of_int end_ln) in
  List.iteri (Errors.read_lines path) ~f:(fun ln str ->
      if ln < ctxt_start_ln || ln > ctxt_end_ln then
        (* This line isn't in our containing span so skip it *)
        ()
      else if
        (ln = ctxt_start_ln || ln = ctxt_end_ln)
        && String.(is_empty @@ lstrip str)
      then
        (* If the line is blank and is either the first of last line, skip it *)
        ()
      else begin
        (* First write the line number and gutter marker  *)
        let ln_num =
          String.pad_left (Int.to_string (ln + 1)) ~char:' ' ~len:line_num_width
        in
        let (_ : unit) =
          Buffer.add_string buf ln_num;
          Buffer.add_string buf " | "
        in
        let is_start = ln = start_ln and is_end = ln = end_ln in
        let (_ : unit) =
          if is_start && is_end then (
            (* If our target pos starts and ends on the line..
               - write the prefix before the start of the marked pos
               - write the lhs cursor marker
               - write the marked string
               - write the rhs cursor marker
               - write the suffix after the end of the marked pos
            *)
            Buffer.add_string buf (String.prefix str start_col);
            Buffer.add_string buf cursor_l;
            Buffer.add_string buf (String.slice str start_col (end_col + 1));
            Buffer.add_string buf cursor_r;
            Buffer.add_string buf (String.drop_prefix str (end_col + 1))
          ) else if is_start then (
            (* If this is the start line, write the prefix, the lhs cursor and
               substring of the marked pos that is on this line *)
            Buffer.add_string buf (String.prefix str start_col);
            Buffer.add_string buf cursor_l;
            Buffer.add_string buf (String.drop_prefix str start_col)
          ) else if is_end then (
            (* If this is the end line, write the substring of the marked pos
               that is on the line, the rhs cursor and the suffix *)
            Buffer.add_string buf (String.prefix str end_col);
            Buffer.add_string buf cursor_r;
            Buffer.add_string buf (String.drop_prefix str end_col)
          ) else if ln < start_ln || ln > end_ln then
            (* Either the whole line is part of the target pos or the target
               is not on the line at all; either way, write the entire line *)
            Buffer.add_string buf str
          else
            Buffer.add_string buf str
        in
        Buffer.add_string buf "\n"
      end)

let user_prompt_prefix buf ctxt_pos user_error =
  let claim_pos =
    Message.get_message_pos (User_error.claim_message user_error)
  in
  Buffer.add_string
    buf
    "Given the following snippet of Hack code that is part of the file:\n<SNIPPET>\n```hack\n";
  extract_prompt_context buf ~ctxt_pos ~claim_pos;
  Buffer.add_string buf "```\n</SNIPPET>\n"

let user_prompt_suffix buf =
  Buffer.add_string
    buf
    {|Edit <SNIPPET> in a way that would fix that lint.
   If there are multiple ways to fix this issue, please return in the code section the most strightforward one that is part of <SNIPPET>,
   any further suggestions can be added in the explanation section.|}

let extended_diagnostics buf user_error =
  Buffer.add_string buf "<DIAGNOSTIC>\n";
  Buffer.add_string buf (Extended_error_formatter.to_string user_error);
  Buffer.add_string buf "\n</DIAGNOSTIC>\n";
  match User_error.custom_errors user_error with
  | [] -> ()
  | msgs ->
    List.iter msgs ~f:(fun str ->
        Buffer.add_string buf (Format.sprintf "<HINT>\n%s\n</HINT>\n" str))

let create_user_prompt selection user_error =
  let buf = Buffer.create 500 in
  user_prompt_prefix buf selection user_error;
  extended_diagnostics buf user_error;
  user_prompt_suffix buf;
  Buffer.contents buf

let legacy_diagnostics buf user_error =
  Buffer.add_string buf "<DIAGNOSTIC>\n";
  Buffer.add_string buf (snd (User_error.claim_message user_error));
  Buffer.add_string buf "\n</DIAGNOSTIC>\n";
  match User_error.reason_messages user_error with
  | [] -> ()
  | msgs ->
    List.iter msgs ~f:(fun (pos, str) ->
        Buffer.add_string
          buf
          (Format.sprintf
             "<HINT>\n%s\nlocation uri:%s\n</HINT>\n"
             str
             (Pos.filename pos)))

let create_legacy_user_prompt selection user_error =
  let buf = Buffer.create 500 in
  user_prompt_prefix buf selection user_error;
  legacy_diagnostics buf user_error;
  user_prompt_suffix buf;
  Buffer.contents buf

let error_to_show_inline_chat_command user_error line_agnostic_hash =
  let claim = User_error.claim_message user_error in
  let override_selection =
    let default = fst claim in
    Option.value_map ~default ~f:(fun pos ->
        let start_line = Pos.line pos and end_line = Pos.end_line pos in
        (* If we aren't in a function the env contains Pos.none - use the diagnostic position in this case *)
        if end_line = 0 then
          default
        (* If size of snippet is greater than our max just use the diagnostic position *)
        else if end_line - start_line >= max_file_content_lines then
          default
        else
          pos)
    @@ User_error.function_pos user_error
  in
  (* LSP uses 0-based line numbers *)
  let webview_start_line = Pos.line override_selection - 1 in
  let display_prompt = Format.sprintf {|Fix inline - %s|} (snd claim) in
  let user_prompt = create_user_prompt override_selection user_error in
  let legacy_user_prompt =
    create_legacy_user_prompt override_selection user_error
  in
  let predefined_prompt =
    Code_action_types.(
      Show_inline_chat_command_args.
        {
          command = "Fix Hack error inline";
          description = Some "Fix Hack error inline";
          display_prompt;
          user_prompt;
          model = Some CODE_31;
          rules = None;
          task = None;
          prompt_template = None;
          add_diagnostics = None;
        })
  in
  let extras =
    Hh_json.(
      JSON_Object
        [
          ("lineAgnosticHash", string_ (Printf.sprintf "%x" line_agnostic_hash));
          ("legacyUserPrompt", string_ legacy_user_prompt);
        ])
  in
  let command_args =
    Code_action_types.(
      Show_inline_chat
        Show_inline_chat_command_args.
          {
            entrypoint = "FixLintErrorCodeAction";
            predefined_prompt;
            override_selection;
            webview_start_line;
            extras;
          })
  in
  let title = Format.sprintf {|Fix Hack error inline - %s|} (snd claim) in
  Code_action_types.{ title; command_args }

let errors_to_commands errors selection =
  List.filter_map
    (Errors.get_error_list ~drop_fixmed:false errors)
    ~f:(fun user_error ->
      if Pos.contains (User_error.get_pos user_error) selection then
        let line_agnostic_hash =
          User_error.hash_error_for_saved_state user_error
        and finalized_error = User_error.to_absolute user_error in
        Some
          (error_to_show_inline_chat_command finalized_error line_agnostic_hash)
      else
        None)

let generate_simplihack_commands ctx tast pos =
  let prompts =
    Simplihack_prompt.find ctx tast.Tast_with_dynamic.under_normal_assumptions
  in
  let create_code_action
      Simplihack_prompt.{ attribute_pos; derive_prompt; edit_span } =
    let open Option.Let_syntax in
    (* Convert the relative position to an absolute position *)
    let attribute_pos = Pos.to_absolute attribute_pos in
    let* derived_prompt = derive_prompt () in
    let user_prompt =
      Format.sprintf
        {| Edit <selection_to_edit> in the following way:
        %s

        Do exactly as instructed above. Do not make any assumptions on what to do unless specifically instructed to do so.
      |}
        derived_prompt
    in
    let predefined_prompt =
      Code_action_types.Show_inline_chat_command_args.
        {
          command = "SimpliHack";
          description = Some "Handle __SimpliHack attribute";
          display_prompt = derived_prompt;
          user_prompt;
          model = Some CODE_31;
          rules = None;
          task = None;
          prompt_template = None;
          add_diagnostics = None;
        }
    in
    (* Calculate the webview start line (LSP uses 0-based line numbers) *)
    let webview_start_line = Pos.line attribute_pos - 1 in
    let command_args =
      Code_action_types.(
        Show_inline_chat
          Show_inline_chat_command_args.
            {
              entrypoint = "HandleUserAttributeCodeAction";
              predefined_prompt;
              override_selection = Pos.to_absolute edit_span;
              webview_start_line;
              extras = Hh_json.JSON_Object [];
            })
    in
    return
      Code_action_types.{ title = "Generate Code for SimpliHack"; command_args }
  in
  let (cursor_line, cursor_col) = Pos.line_column pos in
  List.filter_map
    ~f:(fun prompt ->
      if
        Pos.inside_one_based
          prompt.Simplihack_prompt.attribute_pos
          cursor_line
          cursor_col
      then
        create_code_action prompt
      else
        None)
    prompts

let find ~entry pos ctx ~error_filter : Code_action_types.command list =
  let { Tast_provider.Compute_tast_and_errors.errors; tast; telemetry = _ } =
    Tast_provider.compute_tast_and_errors_quarantined ~ctx ~entry ~error_filter
  in
  let error_commands = errors_to_commands errors pos in
  let simplihack_commands = generate_simplihack_commands ctx tast pos in
  error_commands @ simplihack_commands
