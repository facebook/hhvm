(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections
open Oxidized_module

(** Recursively fetch all exported types in all included modules for re-export,
    since there is no glob-export mechanism in Rust. *)
let get_includes modules includes =
  let get_all_exported_types directly_included_mod acc =
    let rec aux mod_name acc =
      let included_module = SMap.find modules mod_name in
      let acc = SSet.fold included_module.includes ~init:acc ~f:aux in
      let ty_uses = List.map included_module.ty_uses snd in
      let decls = List.map included_module.decls fst in
      let acc = List.fold_right ty_uses ~init:acc ~f:List.cons in
      let acc = List.fold_right decls ~init:acc ~f:List.cons in
      acc
    in
    aux directly_included_mod []
    |> List.dedup_and_sort ~compare
    |> List.fold ~init:acc ~f:(fun acc ty ->
           (directly_included_mod, ty) :: acc)
  in
  SSet.fold includes ~init:[] ~f:get_all_exported_types

let postprocess modules uses aliases includes =
  let includes = get_includes modules includes in
  let uses = List.fold includes ~init:uses ~f:(fun u (m, _) -> SSet.add u m) in
  let uses =
    uses
    |> SSet.elements
    |> List.filter ~f:(fun m ->
           not (List.exists aliases (fun (_, a) -> m = a)))
  in
  (uses, includes)

let stringify modules m =
  let { uses; extern_uses; glob_uses; aliases; includes; ty_uses; decls } =
    m
  in
  let (uses, includes) = postprocess modules uses aliases includes in
  let extern_uses =
    extern_uses
    |> SSet.elements
    |> List.map ~f:(sprintf "use %s;")
    |> String.concat ~sep:"\n"
  in
  let uses =
    uses |> List.map ~f:(sprintf "use crate::%s;") |> String.concat ~sep:"\n"
  in
  let glob_uses =
    glob_uses
    |> SSet.elements
    |> List.map ~f:(sprintf "use crate::%s::*;")
    |> String.concat ~sep:"\n"
  in
  let aliases =
    aliases
    |> List.map ~f:(fun (m, a) -> sprintf "pub use crate::%s as %s;" m a)
    |> String.concat ~sep:"\n"
  in
  let includes =
    includes
    |> List.map ~f:(fun (m, t) -> sprintf "pub use %s::%s;" m t)
    |> String.concat ~sep:"\n"
  in
  let ty_uses =
    ty_uses
    |> List.map ~f:fst
    |> List.map ~f:(sprintf "pub use %s;")
    |> List.dedup_and_sort ~compare
    |> String.concat ~sep:"\n"
  in
  let decls = decls |> List.rev_map ~f:snd |> String.concat ~sep:"\n\n" in
  sprintf
    "%s\n\n%s\n\n%s\n\n%s\n\n%s%s\n\n%s\n"
    extern_uses
    uses
    glob_uses
    aliases
    includes
    ty_uses
    decls
