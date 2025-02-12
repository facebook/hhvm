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

module Show_inline_chat_command_args = struct
  type predefined_prompt = {
    command: string;
    display_prompt: string;
    user_prompt: string;
    description: string option;
    rules: string option;
    task: string option;
    prompt_template: string option;
    model: string option;
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
               field_opt "model" string_ model;
               field_opt "addDiagnostics" bool_ add_diagnostics;
             ]))

  type t = {
    entrypoint: string;
    predefined_prompt: predefined_prompt;
    override_selection: Pos.absolute;
    webview_start_line: int;
  }

  let to_json
      { entrypoint; predefined_prompt; override_selection; webview_start_line }
      =
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
        ])
end

type command_args = Show_inline_chat of Show_inline_chat_command_args.t

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
    let top_down =
      Pass.
        {
          id with
          on_ctor_ty__Tunion =
            Some
              (fun tys ~ctx ->
                let tys =
                  match List.filter ~f:is_not_dynamic tys with
                  | [] -> tys
                  | tys -> tys
                in
                (ctx, `Continue tys));
        }
    in
    let bottom_up = id in
    transform_ty_locl_ty ty ~ctx:() ~top_down ~bottom_up

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
