(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val load_and_parse :
  ?pkgs_config_abs_path:string option -> unit -> Errors.t * Package.Info.t

val repo_config_path : Relative_path.t
