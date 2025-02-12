(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let max_file_content_lines = 400

let extract_prompt_context pos =
  let path = Pos.(filename pos) in
  let start_ln = Pos.line pos - 1 and end_ln = Pos.end_line pos - 1 in
  let line_num_width = 1 + (String.length @@ string_of_int end_ln) in
  let buf = Buffer.create 500 in
  let () =
    List.iteri (Errors.read_lines path) ~f:(fun ln str ->
        if ln >= start_ln && ln <= end_ln then (
          let ln_num =
            String.pad_left
              (Int.to_string (ln + 1))
              ~char:' '
              ~len:line_num_width
          in
          Buffer.add_string buf ln_num;
          Buffer.add_string buf " | ";
          Buffer.add_string buf str;
          Buffer.add_string buf "\n"
        ))
  in
  Buffer.contents buf

let create_user_prompt selection user_error =
  Format.sprintf
    {|
Given the following snippet of Hack code that is part of the file:
%s
<DIAGNOSTIC>
%s
</DIAGNOSTIC>
Edit <selection_to_edit> in a way that would fix that lint.
If there are multiple ways to fix this issue, please return in the code section the most strightforward one that is part of <selection_to_edit>,
any further suggestions can be added in the explanation section.
|}
    (extract_prompt_context selection)
    (Extended_error_formatter.to_string user_error)

let error_to_show_inline_chat_command user_error =
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
  let predefined_prompt =
    Code_action_types.Show_inline_chat_command_args.
      {
        command = "Fix Hack error inline";
        description = Some "Fix Hack error inline";
        display_prompt;
        user_prompt;
        model = Some "iCodeLlama 3.1 405B";
        rules = None;
        task = None;
        prompt_template = None;
        add_diagnostics = None;
      }
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
          })
  in
  let title = Format.sprintf {|Fix Hack error inline - %s|} (snd claim) in
  Code_action_types.{ title; command_args }

let errors_to_commands errors selection =
  List.filter_map
    ~f:(fun user_error ->
      if Pos.contains (User_error.get_pos user_error) selection then
        Some
          (error_to_show_inline_chat_command
          @@ User_error.to_absolute user_error)
      else
        None)
    (Errors.get_error_list ~drop_fixmed:false errors)

let find ~entry pos ctx ~error_filter : Code_action_types.command list =
  let { Tast_provider.Compute_tast_and_errors.errors; _ } =
    Tast_provider.compute_tast_and_errors_quarantined ~ctx ~entry ~error_filter
  in
  errors_to_commands errors pos
