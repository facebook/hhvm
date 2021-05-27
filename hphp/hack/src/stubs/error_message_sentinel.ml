(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let remediation_message =
  Printf.sprintf
    "Addressing other errors or restarting the typechecker with %s might resolve this error."
    (Markdown_lite.md_codify "hh restart")

let please_file_a_bug_message =
  "If remediation attempts don't work, please file a bug report."
