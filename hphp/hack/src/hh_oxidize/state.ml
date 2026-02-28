(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core

let curr_module_name_ref = ref None

let curr_mli_signature_ref = ref None

let curr_module_name () = Option.value_exn !curr_module_name_ref

let curr_mli_signature () = !curr_mli_signature_ref

let with_module_name
    (s : string) ~(mli_signature : Parsetree.signature option) (f : unit -> 'a)
    : 'a =
  try
    assert (not (String.is_empty s));
    assert (Option.is_none !curr_module_name_ref);
    curr_module_name_ref := Some s;
    curr_mli_signature_ref := mli_signature;
    let res = f () in
    curr_module_name_ref := None;
    curr_mli_signature_ref := None;
    res
  with
  | exn ->
    curr_module_name_ref := None;
    curr_mli_signature_ref := None;
    raise exn

let self_ref = ref None

let self () = Option.value_exn !self_ref

let with_self s f =
  try
    assert (not (String.is_empty s));
    assert (Option.is_none !self_ref);
    self_ref := Some s;
    let res = f () in
    self_ref := None;
    res
  with
  | exn ->
    self_ref := None;
    raise exn
