(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

let curr_module_name_ref = ref None

let curr_module_name () = Option.value_exn !curr_module_name_ref

let with_module_name s f =
  try
    assert (s <> "");
    assert (!curr_module_name_ref = None);
    curr_module_name_ref := Some s;
    let res = f () in
    curr_module_name_ref := None;
    res
  with exn ->
    curr_module_name_ref := None;
    raise exn

let self_ref = ref None

let self () = Option.value_exn !self_ref

let with_self s f =
  try
    assert (s <> "");
    assert (!self_ref = None);
    self_ref := Some s;
    let res = f () in
    self_ref := None;
    res
  with exn ->
    self_ref := None;
    raise exn
