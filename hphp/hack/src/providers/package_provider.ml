(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Reduces a multifile-test path [<container>--<simulated/path.php>] to the path
   it simulates. *)
let normalize_path (ctx : Provider_context.t) (path : string) : string =
  let popt = Provider_context.get_popt ctx in
  if popt.ParserOptions.package_support_multifile_tests then
    Multifile.strip_multifile_prefix path
  else
    path

let get_package_for_file (ctx : Provider_context.t) ~(path : string) :
    Package.t option =
  Package_info.get_package_for_file
    (Provider_context.get_package_info ctx)
    ~path:(normalize_path ctx path)

let get_package_with_override_for_file_no_env
    (ctx : Provider_context.t) ~(path : string) ~(content : string) :
    Package.t option * bool =
  Package_info.get_package_with_override_for_file_no_env
    (Provider_context.get_package_info ctx)
    ~path:(normalize_path ctx path)
    ~content
