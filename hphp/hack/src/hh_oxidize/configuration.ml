(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core

type t = {
  extern_types: string S_map.t;
  copy_types: S_set.t option;
  safe_ints_types: S_set.t;
      (** Types for which any ocaml int will be converted to ocamlrep::OCamlInt rather than isize *)
}

let default =
  {
    extern_types = S_map.empty;
    copy_types = None;
    safe_ints_types = S_set.empty;
  }

let config : t option ref = ref None

let set t =
  if Option.is_some !config then failwith "Config already set";
  config := Some t

let extern_type type_name =
  "" :: State.curr_module_name () :: Output.glob_uses ()
  |> List.find_map ~f:(fun mod_name ->
         let maybe_qualified_type =
           if String.equal mod_name "" then
             type_name
           else
             mod_name ^ "::" ^ type_name
         in
         S_map.find_opt
           maybe_qualified_type
           (Option.value_exn !config).extern_types)

let copy_type type_name =
  match (Option.value_exn !config).copy_types with
  | None -> `Unknown
  | Some copy_types ->
    `Known
      ("" :: State.curr_module_name () :: Output.glob_uses ()
      |> List.exists ~f:(fun mod_name ->
             let maybe_qualified_type =
               if String.equal mod_name "" then
                 type_name
               else
                 mod_name ^ "::" ^ type_name
             in
             S_set.mem maybe_qualified_type copy_types))

let is_known v b =
  match v with
  | `Known k -> Bool.equal b k
  | _ -> false

let safe_ints ~mod_name ~name =
  S_set.mem
    (Printf.sprintf "%s::%s" mod_name name)
    (Option.value_exn !config).safe_ints_types
