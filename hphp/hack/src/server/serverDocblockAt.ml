(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let go _env filename line =
  let relative_path = Relative_path.(create Root filename) in
  Docblock_finder.find_single_docblock ~tidy:true relative_path line
