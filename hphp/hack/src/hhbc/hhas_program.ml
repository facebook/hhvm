(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type t = {
  hhas_adata       : Hhas_adata.t list;
  hhas_fun         : Hhas_function.t list;
  hhas_classes     : Hhas_class.t list;
  hhas_typedefs    : Hhas_typedef.t list;
  hhas_main        : Hhas_body.t;
  hhas_symbol_refs : Hhas_symbol_refs.t;
}

let make hhas_adata hhas_fun hhas_classes hhas_typedefs hhas_main hhas_symbol_refs =
  { hhas_adata; hhas_fun; hhas_classes; hhas_typedefs; hhas_main; hhas_symbol_refs; }

let functions hhas_prog =
  hhas_prog.hhas_fun

let classes hhas_prog =
  hhas_prog.hhas_classes

let typedefs hhas_prog =
  hhas_prog.hhas_typedefs

let main hhas_prog =
  hhas_prog.hhas_main

let adata hhas_prog =
  hhas_prog.hhas_adata

let symbol_refs hhas_prog =
  hhas_prog.hhas_symbol_refs

let with_main hhas_prog hhas_main =
  {hhas_prog with hhas_main}

let with_fun hhas_prog hhas_fun =
  {hhas_prog with hhas_fun}

let with_classes hhas_prog hhas_classes =
  {hhas_prog with hhas_classes}

let with_typedefs hhas_prog hhas_typedefs =
  {hhas_prog with hhas_typedefs}

let with_adata hhas_prog hhas_adata =
  {hhas_prog with hhas_adata}
