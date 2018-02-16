(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type hover_info = {
  (** For fields and locals, this is the type. For method and function calls, it
      is the signature, including any inferred types for generics.
      This is also the only part of the hover info to get displayed as code
      rather than Markdown. *)
  snippet : string;

  (** Additional information, such as doc string and declaration file. Displayed
      as Markdown. *)
  addendum : string list;
}

type result = hover_info list

let string_of_result { snippet; addendum } =
  Printf.sprintf "{ snippet = \"%s\"; addendum = [%s] }"
    snippet
    (String.concat "; " (List.map (fun s -> Printf.sprintf "\"%s\"" s) addendum))
