(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type hover_info = {
  snippet: Lsp.markedString list;
  addendum: Lsp.markedString list;
  pos: Pos.t option;  (** Position of this result. *)
}
[@@deriving eq]

type result = hover_info list

let info_as_marked_string_list { snippet; addendum; pos = _ } =
  match snippet with
  | [] ->
    (* Hack server uses None to indicate absence of a result.
       We're also catching the non-result [] just in case... *)
    []
  | _ :: _ ->
    let addendum =
      if List.is_empty addendum then
        addendum
      else
        Lsp.MarkedString "---" :: addendum
    in
    snippet @ addendum

let as_marked_string_list (l : result) =
  List.map l ~f:info_as_marked_string_list
  |> List.intersperse ~sep:[Lsp.MarkedString "---"]
  |> List.concat

let string_of_result { snippet; addendum; pos } =
  Printf.sprintf
    "{ snippet = [%s]; addendum = [%s]; pos = %s }"
    (String.concat ~sep:"; " (List.map ~f:Lsp.show_markedString snippet))
    (String.concat ~sep:"; " (List.map ~f:Lsp.show_markedString addendum))
    (match pos with
    | None -> "None"
    | Some p -> Printf.sprintf "Some %S" (Pos.multiline_string_no_file p))
