(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections

type mode =
  | ByBox
  | ByRef

type t = {
  mode: mode;
  extern_types: string SMap.t;
  owned_types: SSet.t;
  copy_types: SSet.t option;
}

let default =
  {
    extern_types = SMap.empty;
    mode = ByBox;
    owned_types = SSet.empty;
    copy_types = None;
  }

let config : t option ref = ref None

let set t =
  if Option.is_some !config then failwith "Config already set";
  config := Some t

let mode () = (Option.value_exn !config).mode

let extern_type type_name =
  "" :: State.curr_module_name () :: Output.glob_uses ()
  |> List.find_map ~f:(fun mod_name ->
         let maybe_qualified_type =
           if mod_name = "" then
             type_name
           else
             mod_name ^ "::" ^ type_name
         in
         SMap.find_opt
           (Option.value_exn !config).extern_types
           maybe_qualified_type)

let owned_type type_name =
  "" :: State.curr_module_name () :: Output.glob_uses ()
  |> List.exists ~f:(fun mod_name ->
         let maybe_qualified_type =
           if mod_name = "" then
             type_name
           else
             mod_name ^ "::" ^ type_name
         in
         SSet.mem (Option.value_exn !config).owned_types maybe_qualified_type)

let copy_type type_name =
  match (Option.value_exn !config).copy_types with
  | None -> `Unknown
  | Some copy_types ->
    `Known
      ( "" :: State.curr_module_name () :: Output.glob_uses ()
      |> List.exists ~f:(fun mod_name ->
             let maybe_qualified_type =
               if mod_name = "" then
                 type_name
               else
                 mod_name ^ "::" ^ type_name
             in
             SSet.mem copy_types maybe_qualified_type) )
