(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

type t = {
  hhas_is_hh           : bool;
  hhas_adata           : Hhas_adata.t list;
  hhas_fun             : Hhas_function.t list;
  hhas_classes         : Hhas_class.t list;
  hhas_typedefs        : Hhas_typedef.t list;
  hhas_file_attributes : Hhas_attribute.t list;
  hhas_main            : Hhas_body.t;
  hhas_symbol_refs     : Hhas_symbol_refs.t;
  hhas_strict_types    : bool option;
}

let make hhas_is_hh hhas_adata hhas_fun hhas_classes hhas_typedefs
  hhas_file_attributes hhas_main hhas_symbol_refs hhas_strict_types =
  { hhas_is_hh; hhas_adata; hhas_fun; hhas_classes; hhas_typedefs;
    hhas_file_attributes; hhas_main; hhas_symbol_refs; hhas_strict_types; }

let is_hh hhas_prog =
  hhas_prog.hhas_is_hh

let functions hhas_prog =
  hhas_prog.hhas_fun

let classes hhas_prog =
  hhas_prog.hhas_classes

let typedefs hhas_prog =
  hhas_prog.hhas_typedefs

let file_attributes hhas_prog =
  hhas_prog.hhas_file_attributes

let main hhas_prog =
  hhas_prog.hhas_main

let adata hhas_prog =
  hhas_prog.hhas_adata

let symbol_refs hhas_prog =
  hhas_prog.hhas_symbol_refs

let strict_types hhas_prog =
  hhas_prog.hhas_strict_types

let with_main hhas_prog hhas_main =
  {hhas_prog with hhas_main}

let with_fun hhas_prog hhas_fun =
  {hhas_prog with hhas_fun}

let with_classes hhas_prog hhas_classes =
  {hhas_prog with hhas_classes}
