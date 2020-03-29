(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external configs_to_json_ffi : string list -> string list -> string
  = "configs_to_json_ffi"

let from_configs ~(jsons : string list) ~(args : string list) : Hhbc_options.t =
  let merged = configs_to_json_ffi jsons args in
  Hhbc_options.extract_config_options_from_json
    ~init:Hhbc_options.default
    (Some (Hh_json.json_of_string merged))

let get_default () = from_configs [] []
