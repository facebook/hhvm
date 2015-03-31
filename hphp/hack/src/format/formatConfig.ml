(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

type t = {
  indent_offset        : int  ;
  line_width           : int  ;
  indent_case          : bool ;
  align_cascaded_calls : bool ;
}

let default_config = {
  indent_offset        = 2     ;
  line_width           = 80    ;
  indent_case          = true  ;
  align_cascaded_calls = false ;
}

let load_int map key default = match SMap.get key map with
  | None -> default
  | Some s -> int_of_string s

let load_bool map key default = match SMap.get key map with
  | None -> default
  | Some s -> bool_of_string s

let load config_filename =
  let config_path = (Relative_path.to_absolute config_filename) in
  if Path.file_exists (Path.mk_path config_path) then
    let config = Config_file.parse config_path in

    let indent_offset        =
      load_int config "format.indent_offset" default_config.indent_offset in
    let line_width           =
      load_int config "format.line_width" default_config.line_width in
    let indent_case          =
      load_bool config "format.indent_case" default_config.indent_case in
    let align_cascaded_calls =
      load_bool config "format.align_cascaded_calls"
                default_config.align_cascaded_calls in

    {
      indent_offset        = indent_offset;
      line_width           = line_width;
      indent_case          = indent_case;
      align_cascaded_calls = align_cascaded_calls;
    }
  else default_config

let indent_offset config = config.indent_offset
let line_width config = config.line_width
let indent_case config = config.indent_case
let align_cascaded_calls config = config.align_cascaded_calls
