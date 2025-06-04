(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type edit = {
  pos: Pos.t;
  text: string;
}

type edits = edit list Relative_path.Map.t

type edit_data = {
  title: string;
  edits: edits Lazy.t;
  selection: Pos.t option;
  trigger_inline_suggest: bool;
}

type refactor = Refactor of edit_data [@@ocaml.unboxed]

type quickfix = Quickfix of edit_data [@@ocaml.unboxed]

module Show_sidebar_chat_command_args = struct
  type message_attachment = {
    title: string;
    value: string;
  }

  let message_attachment_to_json { title; value } =
    Hh_json.(JSON_Object [("title", string_ title); ("value", string_ value)])

  type message_model =
    | GPT4
    | ILLAMA
    | ICODELLAMA_TEST
    | ICODELLAMA_405B
    | CLAUDE_37_SONNET

  let message_model_to_json = function
    | GPT4 -> Hh_json.string_ "GPT4"
    | ILLAMA -> Hh_json.string_ "ILLAMA"
    | ICODELLAMA_TEST -> Hh_json.string_ "ICODELLAMA_TEST"
    | ICODELLAMA_405B -> Hh_json.string_ "ICODELLAMA_405B"
    | CLAUDE_37_SONNET -> Hh_json.string_ "CLAUDE_37_SONNET"

  type action =
    | CHAT  (** Chat (user triggered) *)
    | CODE  (** Code action  *)
    | DEVMATE  (** Devmate  *)
    | EXAMPLE  (** An example prompt *)

  let action_to_json = function
    | CHAT -> Hh_json.string_ "CHAT"
    | CODE -> Hh_json.string_ "CODE"
    | DEVMATE -> Hh_json.string_ "DEVMATE"
    | EXAMPLE -> Hh_json.string_ "EXAMPLE"

  type action_trigger_type =
    | Code_action
    | Context_menu
    | Example_prompt
    | Diagnostic_hover
    | Slash_command
    | Code_lens

  let action_trigger_type_to_json = function
    | Code_action -> Hh_json.string_ "CodeAction"
    | Context_menu -> Hh_json.string_ "ContextMenu"
    | Example_prompt -> Hh_json.string_ "ExamplePrompt"
    | Diagnostic_hover -> Hh_json.string_ "DiagnosticHover"
    | Slash_command -> Hh_json.string_ "SlashCommand"
    | Code_lens -> Hh_json.string_ "CodeLens"

  type llm_config_model =
    | G4
    | ICODELLAMA_TEST
    | ICODELLAMA_405B
    | CLAUDE_37_SONNET

  let llm_config_model_to_json = function
    | G4 -> Hh_json.string_ "g4"
    | ICODELLAMA_TEST -> Hh_json.string_ "icodellama-test"
    | ICODELLAMA_405B -> Hh_json.string_ "icodellama-405b"
    | CLAUDE_37_SONNET -> Hh_json.string_ "claude3.7-sonnet"

  type tool_call_results_type = {
    tool_call_id: string;
    tool_name: string;
    arguments: string;
    output: string;
  }

  let tool_call_results_type_to_json
      { tool_call_id; tool_name; arguments; output } =
    Hh_json.(
      JSON_Object
        [
          ("tool_call_id", string_ tool_call_id);
          ("tool_name", string_ tool_name);
          ("arguments", string_ arguments);
          ("output", string_ output);
        ])

  type model_params = {
    model: llm_config_model option;
    max_tokens: int option;
    temperature: float option;
    top_p: float option;
    stop: string list option;
    repetition_penalty: float option;
  }

  let model_params_to_json
      { model; max_tokens; temperature; top_p; stop; repetition_penalty } =
    Hh_json.(
      JSON_Object
        (List.filter_opt
           [
             Option.map model ~f:(fun model ->
                 ("model", llm_config_model_to_json model));
             Option.map max_tokens ~f:(fun max_tokens ->
                 ("max_tokens", int_ max_tokens));
             Option.map temperature ~f:(fun temperature ->
                 ("temperature", float_ temperature));
             Option.map top_p ~f:(fun top_p -> ("top_p", float_ top_p));
             Option.map stop ~f:(fun stop -> ("stop", array_ string_ stop));
             Option.map repetition_penalty ~f:(fun repetition_penalty ->
                 ("repetition_penalty", float_ repetition_penalty));
           ]))

  type llm_config = {
    pipeline: string option;
    label: string option;
    model_params: model_params option;
    max_input_token: int option;
  }

  let llm_config_to_json { pipeline; label; model_params; max_input_token } =
    Hh_json.(
      JSON_Object
        (List.filter_opt
           [
             Option.map pipeline ~f:(fun pipeline ->
                 ("pipeline", string_ pipeline));
             Option.map label ~f:(fun label -> ("label", string_ label));
             Option.map model_params ~f:(fun model_params ->
                 ("model_params", model_params_to_json model_params));
             Option.map max_input_token ~f:(fun max_input_token ->
                 ("max_input_token", int_ max_input_token));
           ]))

  type t = {
    display_prompt: string;
        (** The prompt written by/to be displayed to the user. *)
    model_prompt: string option;
        (** The actual prompt that will be sent to the LLM. *)
    llm_config: llm_config option;
        (** Override the default LLM config for this prompt. *)
    model: message_model option;
    attachments: message_attachment list option;
    action: action;  (** The action that triggered this prompt. *)
    trigger: string option;
        (** Where this prompt was triggered from.
      For example, this could be which code action was used. *)
    trigger_type: action_trigger_type option;
        (** The type of action that triggered this prompt.
      For example, this could be a in context code action or a slash command. *)
    local_tool_results: tool_call_results_type option;
        (** The results of the local tool calls run. *)
    correlation_id: string option;
        (** The unique identifier for this request provided from the calling language server *)
  }

  let to_json
      {
        display_prompt;
        model_prompt;
        llm_config;
        model;
        attachments;
        action;
        trigger;
        trigger_type;
        local_tool_results;
        correlation_id;
      } =
    Hh_json.(
      JSON_Object
        (List.filter_opt
           [
             Some ("displayPrompt", string_ display_prompt);
             Option.map model_prompt ~f:(fun model_prompt ->
                 ("modelPrompt", string_ model_prompt));
             Option.map llm_config ~f:(fun llm_config ->
                 ("llmConfig", llm_config_to_json llm_config));
             Option.map model ~f:(fun model ->
                 ("model", message_model_to_json model));
             Option.map attachments ~f:(fun attachments ->
                 ("attachments", array_ message_attachment_to_json attachments));
             Some ("action", action_to_json action);
             Option.map trigger ~f:(fun trigger -> ("trigger", string_ trigger));
             Option.map trigger_type ~f:(fun trigger_type ->
                 ("triggerType", action_trigger_type_to_json trigger_type));
             Option.map local_tool_results ~f:(fun local_tool_results ->
                 ( "localToolResults",
                   tool_call_results_type_to_json local_tool_results ));
             Option.map correlation_id ~f:(fun correlation_id ->
                 ("correlationId", string_ correlation_id));
           ]))
end

module Show_inline_chat_command_args = struct
  type model =
    | GPT4o
    | SONNET_37
    | CODE_31
    | LLAMA_405B

  let model_to_json = function
    | GPT4o -> Hh_json.string_ "GPT-4o"
    | SONNET_37 -> Hh_json.string_ "Claude 3.7 Sonnet"
    | CODE_31 -> Hh_json.string_ "iCodeLlama 3.1 70B"
    | LLAMA_405B -> Hh_json.string_ "iCodeLlama 3.1 405B"

  type predefined_prompt = {
    command: string;
    display_prompt: string;
    user_prompt: string;
    description: string option;
    rules: string option;
    task: string option;
    prompt_template: string option;
    model: model option;
    add_diagnostics: bool option;
  }

  let field_opt name t_to_json t_opt =
    Option.map t_opt ~f:(fun t -> (name, t_to_json t))

  let predefined_prompt_to_json
      {
        command;
        display_prompt;
        user_prompt;
        description;
        rules;
        task;
        prompt_template;
        model;
        add_diagnostics;
      } =
    Hh_json.(
      JSON_Object
        (("command", string_ command)
        :: ("displayPrompt", string_ display_prompt)
        :: ("userPrompt", string_ user_prompt)
        :: List.filter_opt
             [
               field_opt "description" string_ description;
               field_opt "rules" string_ rules;
               field_opt "task" string_ task;
               field_opt "promptTemplate" string_ prompt_template;
               field_opt "model" model_to_json model;
               field_opt "addDiagnostics" bool_ add_diagnostics;
             ]))

  type t = {
    entrypoint: string;
    predefined_prompt: predefined_prompt;
    override_selection: Pos.absolute;
    webview_start_line: int;
    extras: Hh_json.json;
  }

  let to_json
      {
        entrypoint;
        predefined_prompt;
        override_selection;
        webview_start_line;
        extras;
      } =
    Hh_json.(
      JSON_Object
        [
          ("entrypoint", string_ entrypoint);
          ("predefinedPrompt", predefined_prompt_to_json predefined_prompt);
          ( "overrideSelection",
            Lsp_fmt.print_range
            @@ Lsp_helpers.hack_pos_to_lsp_range
                 ~equal:String.equal
                 override_selection );
          ("webviewStartLine", int_ webview_start_line);
          ("extras", extras);
        ])
end

type command_args =
  | Show_inline_chat of Show_inline_chat_command_args.t
  | Show_sidebar_chat of Show_sidebar_chat_command_args.t

type command = {
  title: string;
  command_args: command_args;
}

type t =
  | Refactor_action of edit_data
  | Quickfix_action of edit_data
  | Command_action of command

type find_refactor =
  entry:Provider_context.entry -> Pos.t -> Provider_context.t -> refactor list

type find_quickfix =
  entry:Provider_context.entry ->
  Pos.t ->
  Provider_context.t ->
  error_filter:Tast_provider.ErrorFilter.t ->
  quickfix list

module Type_string = struct
  type t = string Lazy.t

  (** 'like types' are encoded as the union of a static type with [Tdynamic]
       so we can remove them by filtering out [Tdynamic] from the types contained
       in such a [Tunion]. We _shouldn't_ encounter the degenerate union cases
       [Tunion [Tdynamic]] or [Tunion [Tdynamic, ..., Tdynamic]] since they
       should have been simplified but we check for this just in case *)
  let deep_strip_like_types ty =
    let open Typing_defs_core in
    let is_not_dynamic ty =
      match get_node ty with
      | Tdynamic -> false
      | _ -> true
    in
    let id = Pass.identity () in
    let on_tunion :
        type a. a ty list -> ctx:_ -> _ * [> `Continue of a ty list ] =
     fun tys ~ctx ->
      let tys =
        match List.filter ~f:is_not_dynamic tys with
        | [] -> tys
        | tys -> tys
      in
      (ctx, `Continue tys)
    in
    let top_down = Pass.{ id with on_ctor_ty__Tunion = Some on_tunion } in
    let bottom_up = id in
    transform_ty_ty ty ~ctx:() ~top_down ~bottom_up

  (** Don't truncate types in printing unless they are really big,
     so we almost always generate valid code.
     The number is somewhat arbitrary: it's the smallest power of 2
     that could print without truncation for
     extract_shape_type_13.php in our test suite.
     We *do* want to truncate at some finite number so editors can
     handle that much text. *)
  let lots_of_typing_print_fuel = 2048

  let of_locl_ty tast_env locl_ty =
    lazy
      (let typing_env =
         tast_env
         |> Tast_env.tast_env_as_typing_env
         |> Typing_env.map_tcopt ~f:(fun tcopt ->
                {
                  tcopt with
                  GlobalOptions.tco_type_printer_fuel =
                    lots_of_typing_print_fuel;
                })
       in
       locl_ty
       |> Tast_env.fully_expand tast_env
       |> deep_strip_like_types
       |> Typing_print.full_strip_ns ~hide_internals:true typing_env)

  let to_string = Lazy.force
end
