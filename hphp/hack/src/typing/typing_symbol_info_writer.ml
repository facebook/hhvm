(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hh_json

let write_json
  (tast_lst: (Relative_path.t * Tast.program) list)
  (filename_prefix: string)
  : unit =

  (**
    * TODO:
    * - retrieve decl information from tast_lst
    * - format json for Glean
    * - write json file to `filename_prefix` ^ ".json"
    *)

  let _ = tast_lst in (* to prevent errors of unused var *)
  let json_data = JSON_Array([]) in (* empty json data *)
  let filename = filename_prefix ^ ".json" in
  let channel = Out_channel.create ~fail_if_exists:true filename in

  Out_channel.output_string channel (json_to_string ~pretty:true json_data);
  Out_channel.close channel
