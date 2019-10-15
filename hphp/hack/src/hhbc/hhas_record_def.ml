(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  record_name: Hhbc_id.Record.t;
  record_is_abstract: bool;
  record_base: Hhbc_id.Record.t option;
  record_fields: (string * Hhas_type_info.t * Typed_value.t option) list;
}

let make record_name record_is_abstract record_base record_fields =
  { record_name; record_is_abstract; record_base; record_fields }

let name hhas_record_decl = hhas_record_decl.record_name

let is_abstract hhas_record_decl = hhas_record_decl.record_is_abstract

let base hhas_record_decl = hhas_record_decl.record_base

let fields hhas_record_decl = hhas_record_decl.record_fields
