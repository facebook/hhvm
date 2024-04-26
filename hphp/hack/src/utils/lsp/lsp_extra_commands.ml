(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Command = struct
  type t = {
    title: string;
    command: string;
  }

  let to_lsp (t : t) arguments =
    Lsp.Command.{ title = t.title; command = t.command; arguments }

  let from_lsp
      (t : t)
      (f : Hh_json.json list -> ('a, string) Result.t)
      (lsp_cmd : Lsp.Command.t) : ('a option, string) Result.t =
    if String.equal t.command lsp_cmd.Lsp.Command.command then
      f lsp_cmd.Lsp.Command.arguments
      |> Result.map ~f:(fun x -> Some x)
      |> Result.map_error ~f:(fun error ->
             Printf.sprintf
               "could not parse arguments for command %s: %s"
               t.command
               error)
    else
      Ok None

  let set_selection : t =
    { title = "Set Cursor Selection"; command = "hack.setSelection" }
end

let set_selection (range : Lsp.range) : Lsp.Command.t =
  Command.to_lsp Command.set_selection [Lsp_fmt.print_range range]

let parse_set_selection : Lsp.Command.t -> (Lsp.range option, string) Result.t =
  Command.from_lsp Command.set_selection @@ fun args ->
  match args with
  | [range] -> begin
    try Ok (Lsp_fmt.parse_range_exn (Some range)) with
    | Hh_json_helpers.Jget.Parse error ->
      Error (Printf.sprintf "parse_range_exn: Jget: %s" error)
  end
  | xs ->
    Error
      (Printf.sprintf
         "expected exactly one argument, but got %d"
         (List.length xs))
