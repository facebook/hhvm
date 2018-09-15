(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let go tcopt files_info f_in =
  let check, tast = ServerIdeUtils.check_file_input
  tcopt files_info f_in in
  Coverage_level.get_levels tast check
