(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let ignore_ide_from _ = false

let waiting_for_server_to_be_started_doc = ""

let angery_reaccs_only () = false

let lsp_explanation_for_no_server_running =
  "hh_server isn't running. It's common to leave hh_server running persistently. \
  Click 'Restart' button below when ready."
