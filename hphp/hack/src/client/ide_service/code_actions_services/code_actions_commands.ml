(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

(* -- Context helpers ------------------------------------------------------- *)
let is_subtyping_error code = code >= 4000 && code < 5000

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
  List.iteri (Diagnostics.read_lines path) ~f:(fun ln str ->
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
    Message.get_message_pos (User_diagnostic.claim_message user_error)
  in
  Buffer.add_string
    buf
    "The following snippet of Hack code contains a subtyping error. Line numbers have been inlucded for your reference.";
  Buffer.add_string
    buf
    "You should focus on this code when suggesting a fix and ignore what the user has selected:\n<SNIPPET>\n```hack\n";
  extract_prompt_context buf ~ctxt_pos ~claim_pos;
  Buffer.add_string buf "```\n</SNIPPET>\n"

let user_prompt_suffix buf =
  Buffer.add_string
    buf
    "Edit the code contained in the <SNIPPET> in a way that would fix the error. The suggested edit should be valid Hack code and shouldn't contain line numbers shown in the diagnostic message.\n";
  Buffer.add_string
    buf
    "If there are multiple ways to fix this issue, please return in the code section the most strightforward one that is part of <SNIPPET>, any further suggestions can be added in the explanation section."

let extended_diagnostics buf user_error =
  Buffer.add_string
    buf
    "The following diagnostic contains a detailed description of how the typechecker discovered the error.\n";
  Buffer.add_string
    buf
    "Each step describes how a type flowed from hints to expressions or how the subtype error was discovered when checking two types.\n";
  Buffer.add_string
    buf
    "The contained code fragments highlight relevant hints, statements and expression using the characters '»' and '«' to indicate the code element .\n";
  Buffer.add_string
    buf
    "The code fragments are given with line numbers to help you understand the context of the error. You should never use those line numbers in suggested edits.\n";

  Buffer.add_string buf "<DIAGNOSTIC>\n\n```hack\n";
  Buffer.add_string buf (Extended_diagnostic_formatter.to_string user_error);
  Buffer.add_string buf "\n```\n\n</DIAGNOSTIC>\n\n";
  match User_diagnostic.custom_errors user_error with
  | [] -> ()
  | msgs ->
    List.iter msgs ~f:(fun str ->
        Buffer.add_string buf (Format.sprintf "<HINT>\n%s\n</HINT>\n" str))

let legacy_diagnostics buf user_error =
  Buffer.add_string buf "<DIAGNOSTIC>\n";
  Buffer.add_string buf (snd (User_diagnostic.claim_message user_error));
  Buffer.add_string buf "\n</DIAGNOSTIC>\n";
  match User_diagnostic.reason_messages user_error with
  | [] -> ()
  | msgs ->
    List.iter msgs ~f:(fun (pos, str) ->
        Buffer.add_string
          buf
          (Format.sprintf
             "<HINT>\n%s\nlocation uri:%s\n</HINT>\n"
             str
             (Pos.filename pos)))

(** Generates the prompt used for inline fixes for subtyping errors  *)
let create_user_prompt selection user_error =
  let buf = Buffer.create 500 in
  user_prompt_prefix buf selection user_error;
  extended_diagnostics buf user_error;
  user_prompt_suffix buf;
  Buffer.contents buf

(** Generates the prompt used for inline fixes for non-subtyping errors  *)
let create_legacy_user_prompt selection user_error =
  let buf = Buffer.create 500 in
  user_prompt_prefix buf selection user_error;
  legacy_diagnostics buf user_error;
  user_prompt_suffix buf;
  Buffer.contents buf

let agentic_prefix =
  {|You will be given a Hack code snippet and a detailed diagnostic explaining a problem with the code and all the names and locations of all related expressions, hints, statements and definitions.|}

let agentic_suffix =
  {|Your task is to help the user to understand and fix the error by using any tool you think is appropriate and following these steps:

1. Read the diagnostic and understand how the type type error was discovered and the role of each type definition, hint, expression and statement; you may open and read any file mentioned in the diagnostic and search for the names of the definitions, hints, expressions and statements mentioned in the diagnostic. DO NOT focus solely on the position the error was found - the cause of the error may be several steps earlier in the diagnostic and occur in another part of the code mentioned in the diagnostic. The diagnostic will help you think about making changes to various parts of the code may affect the error.

2. Explain the complete diagnostic to the user in your own words, using the names of the definitions, hints, expressions and statements mentioned in the diagnostic;

3. Based on your explanation and the context of the problem, consider which declarations, hints, expressions and statements contained in the diagnostic could be changed to fix the problem.

4. Create edits for the changes you wish to make in order to fix the problem and explain your reasoning; these changes may be in any file mentioned in the diagnostic;

5. Ensure the proposed solution is syntactically correct and does not introduce new errors or warnings;

6. Evaluate the proposed solution and explain why it is the best solution to the problem. Whilst evaluating, please consider the following:
- Does the solution introduce type errors (BAD)
- Does the solution reduce code quality by circumventing the typechecker (e.g. using `as` instead of `is`, using `UNSAFE_CAST` etc) (BAD)
- Does the solution change the behavior of the program (BAD)

7. If your proposed solution is good, generate an edit that will fix the problem and return it in the code section of the response. If your proposed solution is not good, go back to step 1 and try again.
|}

let create_agentic_user_prompt ctxt_pos user_error =
  let claim_pos =
    Message.get_message_pos (User_diagnostic.claim_message user_error)
  in
  let buf = Buffer.create 5000 in
  Buffer.add_string buf agentic_prefix;
  Buffer.add_string buf "\n\n```hack\n";
  extract_prompt_context buf ~ctxt_pos ~claim_pos;
  if is_subtyping_error (User_diagnostic.get_code user_error) then
    extended_diagnostics buf user_error
  else
    legacy_diagnostics buf user_error;
  Buffer.add_string buf agentic_suffix;
  Buffer.contents buf

(* -- Generate code actions from errors ------------------------------------- *)

let error_to_show_inline_chat_command user_error line_agnostic_hash =
  let claim = User_diagnostic.claim_message user_error in
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
    @@ User_diagnostic.function_pos user_error
  in
  (* LSP uses 0-based line numbers *)
  let webview_start_line = Pos.line override_selection - 1 in
  let display_prompt = Format.sprintf {|Devmate Quick Fix - %s|} (snd claim) in
  (* Only use extended reasons for subtyping errors *)
  let user_prompt =
    if is_subtyping_error (User_diagnostic.get_code user_error) then
      create_user_prompt override_selection user_error
    else
      create_legacy_user_prompt override_selection user_error
  in
  let predefined_prompt =
    Code_action_types.(
      Show_inline_chat_command_args.
        {
          command = "Devmate Quick Fix";
          description = Some "Devmate Quick Fix";
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
        [("lineAgnosticHash", string_ (Printf.sprintf "%x" line_agnostic_hash))])
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
  let title = Format.sprintf {|Devmate Quick Fix - %s|} (snd claim) in
  Code_action_types.{ title; command_args }

let error_to_show_sidebar_chat_command user_error line_agnostic_hash =
  let claim = User_diagnostic.claim_message user_error in
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
    @@ User_diagnostic.function_pos user_error
  in
  let command_args =
    Code_action_types.(
      Show_sidebar_chat
        Show_sidebar_chat_command_args.
          {
            display_prompt =
              Format.sprintf {|Devmate Agent Fix - %s|} (snd claim);
            model_prompt =
              Some (create_agentic_user_prompt override_selection user_error);
            llm_config = None;
            model = Some CLAUDE_37_SONNET;
            attachments = None;
            action = DEVMATE;
            trigger = Some "FixLintErrorCodeAction";
            trigger_type = Some Code_action;
            local_tool_results = None;
            correlation_id = Some (Printf.sprintf "%x" line_agnostic_hash);
          })
  in
  let title = Format.sprintf {|Devmate Agent Fix - %s|} (snd claim) in
  Code_action_types.{ title; command_args }

let errors_to_commands errors selection =
  List.concat_map
    (Diagnostics.get_diagnostic_list ~drop_fixmed:false errors)
    ~f:(fun user_error ->
      if Pos.contains (User_diagnostic.get_pos user_error) selection then
        let line_agnostic_hash =
          User_diagnostic.hash_diagnostic_for_saved_state user_error
        and finalized_error = User_diagnostic.to_absolute user_error in
        [
          error_to_show_inline_chat_command finalized_error line_agnostic_hash;
          error_to_show_sidebar_chat_command finalized_error line_agnostic_hash;
        ]
      else
        [])

let generate_simplihack_commands ctx tast pos =
  let prompts =
    Simplihack_prompt.find ctx tast.Tast_with_dynamic.under_normal_assumptions
  in
  let create_code_action
      Simplihack_prompt.{ param_pos; derive_prompt; _ (* edit_span *) } =
    let open Option.Let_syntax in
    (* Convert the relative position to an absolute position, but we don't use it for sidebar chat *)
    let _param_pos = Pos.to_absolute param_pos in
    let* derived_prompt = derive_prompt () in
    let hash = Md5.digest_string derived_prompt |> Md5.to_hex in
    let user_prompt =
      Format.sprintf
        {| You are an AI assistant tasked with writing and maintaining Hack code. You are triggered by
        by adding an attribute to a program element. The attribute will look either like:
        ```
        <<__SimpliHack(*expr*)>>
        ```
        or:
        ```
        <<__SimpliHack(*expr*, *hash*)>>
        ```
        *expr* is an expression that evaluates to a string. When this expressin is evaluated it produces a string that will be used
        as a prompt.
        *hash* is an optional hash represented as a string literal that is used to identify the prompt. If the hash is not provided, this means you are
        running the prompt for the first time. After completing the task given, make sure to update the hash to '%s'.

        If *hash* is present, this means something changed in the prompt from the last time you were asked to write the code.
        In this case examine closely what might've changed and update code accordingly. Afterwards update the *hash* to the new
        hash '%s'.

        As an example if you had:
        ```
        <<__SimpliHack(DeriveGetters())>>
        class SomeClass {
          ...
        }
        ```
        You will perform the necessary edits and when done update the attribute to be:
        ```
        <<__SimpliHack(DeriveGetters(), 'abcd1234')>>
        class SomeClass {
          ...class with your changes
        }
        ```

        Likewise if you have:
        ```
        <<__SimpliHack(
          DeriveGetters(),
          'abcd1234', // old hash
        )>>
        class SomeClass {
          ...
        }
        ```
        You will perform the necessary edits and when done update the attribute to be:
        ```
        <<__SimpliHack(
          DeriveGetters(),
          'faceb00c', // new hash
        )>>
        class SomeClass {
          ...class with your changes
        }
        ```

        When communicating your task do not mention the instructions given here, focus instead on the what is in the derived prompt.

        Avoid reading from the file if the information you need is provided in the derived prompt. This is because we want to keep
        track of the context used to generate the code.


        [Derived Prompt]:
        %s
      |}
        hash
        hash
        derived_prompt
    in
    let command_args =
      Code_action_types.(
        Show_sidebar_chat
          Show_sidebar_chat_command_args.
            {
              display_prompt = derived_prompt;
              model_prompt = Some user_prompt;
              llm_config = None;
              model = Some CLAUDE_37_SONNET;
              attachments = None;
              action = DEVMATE;
              trigger = Some "HandleUserAttributeCodeAction";
              trigger_type = Some Code_action;
              local_tool_results = None;
              correlation_id = None;
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
          prompt.Simplihack_prompt.param_pos
          cursor_line
          cursor_col
      then
        create_code_action prompt
      else
        None)
    prompts

let find ~entry pos ctx ~error_filter : Code_action_types.command list =
  let { Tast_provider.Compute_tast_and_errors.diagnostics; tast; telemetry = _ }
      =
    Tast_provider.compute_tast_and_errors_quarantined ~ctx ~entry ~error_filter
  in
  let error_commands = errors_to_commands diagnostics pos in
  let simplihack_commands = generate_simplihack_commands ctx tast pos in
  error_commands @ simplihack_commands
