(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
open Reordered_argument_collections

type t = {
  extern_types: string SMap.t;
  copy_types: SSet.t option;
  safe_ints_types: SSet.t;
      (** Types for which any ocaml int will be converted to ocamlrep::OCamlInt rather than isize *)
}

let default =
  { extern_types = SMap.empty; copy_types = None; safe_ints_types = SSet.empty }

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
         SMap.find_opt
           (Option.value_exn !config).extern_types
           maybe_qualified_type)

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
             SSet.mem copy_types maybe_qualified_type))

let is_known v b =
  match v with
  | `Known k -> Bool.equal b k
  | _ -> false

let safe_ints ~mod_name ~name =
  SSet.mem
    (Option.value_exn !config).safe_ints_types
    (Printf.sprintf "%s::%s" mod_name name)
