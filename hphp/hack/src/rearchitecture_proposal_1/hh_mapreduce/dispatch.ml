(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type kind =
  | Prototype
  | Typecheck

type info = {
  name: string;
  kind: kind;
  run: unit -> unit;
  run_worker: Unix.file_descr -> Decl_service_client.t -> unit;
}

let dispatch_list : info list ref = ref []

let register (new_dispatch_list : info list) : unit =
  if !dispatch_list <> [] then failwith "Cannot re-register dispatch_list";
  if new_dispatch_list = [] then failwith "Why an empty dispatch list?";
  dispatch_list := new_dispatch_list

let find_by_kind (kind : kind) : info =
  if !dispatch_list = [] then failwith "Dispatch list not yet registered?";
  let info_opt = List.find !dispatch_list ~f:(fun info -> info.kind = kind) in
  (* All dispatch_kinds should have been registered by now it's a code bug if not *)
  Option.value_exn info_opt

let find_by_name (name : string) : info option =
  if !dispatch_list = [] then failwith "Dispatch list not yet registered?";
  List.find !dispatch_list ~f:(fun info -> info.name = name)
