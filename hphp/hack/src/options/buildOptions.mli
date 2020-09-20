(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Where we look for system-wide configuration files.
 *
 * Possible values include:
 * - "/etc": most linux systems
 * - "/usr/local/etc": MacOS homebrew
 *)
val system_config_path : string

(**
 * Where to look for hackfmt.
 *
 * Possible values include:
 * - "/usr/local/bin/hackfmt": manual builds, Facebook
 * - "/usr/bin/hackfmt": most linux binary builds
 * - "/usr/local/Cellar/hhvm/VERSION/bin/hackfmt": MacOS homebrew
 *)
val default_hackfmt_path : string
