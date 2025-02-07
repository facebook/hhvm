(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type hover_info = {
  snippet: string;
      (** For fields and locals, this is the type. For method and function calls, it
          is the signature, including any inferred types for generics.
          This is also the only part of the hover info to get displayed as code
          rather than Markdown. *)
  addendum: string list;
      (** Additional information, such as doc string and declaration file. Displayed
          as Markdown. *)
  pos: Pos.t option;  (** Position of this result. *)
}
[@@deriving eq]

type result = hover_info list

let string_of_result { snippet; addendum; pos } =
  Printf.sprintf
    "{ snippet = %S; addendum = [%s]; pos = %s }"
    snippet
    (String.concat "; " (List.map (fun s -> Printf.sprintf "%S" s) addendum))
    (match pos with
    | None -> "None"
    | Some p -> Printf.sprintf "Some %S" (Pos.multiline_string_no_file p))
