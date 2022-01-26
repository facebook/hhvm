(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let pf fmt = Format.fprintf fmt

let string = Format.pp_print_string

let sp ppf _ = Format.pp_print_space ppf ()

let cut ppf _ = Format.pp_print_cut ppf ()

let to_to_string pp_v v = Format.asprintf "%a" pp_v v

let of_to_string f ppf v = string ppf (f v)

let cond ~pp_t ~pp_f ppf test =
  if test then
    pp_t ppf ()
  else
    pp_f ppf ()

let pair ?sep:(pp_sep = sp) pp_a pp_b ppf (a, b) =
  pp_a ppf a;
  pp_sep ppf ();
  pp_b ppf b

let prefix pp_pfx pp_v ppf v =
  pp_pfx ppf ();
  pp_v ppf v

let suffix pp_sfx pp_v ppf v =
  pp_v ppf v;
  pp_sfx ppf ()

let surround l r pp_v ppf v =
  string ppf l;
  pp_v ppf v;
  string ppf r

let quote pp_v ppf v = surround "'" "'" pp_v ppf v

let parens pp_v ppf v = surround "(" ")" pp_v ppf v

let braces pp_v ppf v = surround "{" "}" pp_v ppf v

let angles pp_v ppf v = surround "<" ">" pp_v ppf v

let brackets pp_v ppf v = surround "[" "]" pp_v ppf v

let comma ppf _ =
  string ppf ",";
  sp ppf ()

let amp ppf _ =
  sp ppf ();
  string ppf "&";
  sp ppf ()

let equal_to ppf _ =
  sp ppf ();
  string ppf "=";
  sp ppf ()

let colon ppf _ =
  sp ppf ();
  string ppf ":";
  sp ppf ()

let semicolon ppf _ = string ppf ";"

let arrow ppf _ = string ppf "->"

let fat_arrow ppf _ =
  sp ppf ();
  string ppf "=>";
  sp ppf ()

let const str ppf _ = string ppf str

let dbl_colon v = const "::" v

let dbl_hash v = const "#" v

let vbar ppf _ =
  sp ppf ();
  string ppf "|";
  sp ppf ()

let nop _ _ = ()

let const pp_v v ppf _ = pp_v ppf v

let option ?none:(pp_none = nop) pp_v ppf = function
  | Some v -> pp_v ppf v
  | _ -> pp_none ppf ()

let list ?sep:(pp_sep = sp) pp_elt ppf v =
  let is_first = ref true in
  let pp_elt v =
    if !is_first then
      is_first := false
    else
      pp_sep ppf ();
    pp_elt ppf v
  in
  List.iter ~f:pp_elt v

let hbox pp_v ppf v =
  Format.(
    pp_open_hbox ppf ();
    pp_v ppf v;
    pp_close_box ppf ())

let vbox ?(indent = 0) pp_v ppf v =
  Format.(
    pp_open_vbox ppf indent;
    pp_v ppf v;
    pp_close_box ppf ())
