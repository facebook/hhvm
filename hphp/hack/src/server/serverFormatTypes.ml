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
type result = (Ide_message.format_response, string) Result.t

type ide_action = Ide_api_types.file_range
type ide_result = (Ide_message.format_response, string) Result.t
