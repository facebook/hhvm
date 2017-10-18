(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


type action = string * int * int (* file contents, offset start, offset end *)
type result = (Ide_message.format_response, string) Pervasives.result


type ide_action =
  | Document of string  (* filename *)
  | Range of Ide_api_types.file_range
  | Position of Ide_api_types.file_position

type ide_response = {
  new_text: Ide_message.format_response;
  range: Ide_api_types.range; (* what range was actually replaced? *)
}

type ide_result = (ide_response, string) Pervasives.result
