(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ifc_types

let do_ opts files_info ctx =
  (if Ifc.should_print ~user_mode:opts.opt_mode ~phase:Mlattice then
    let lattice = opts.opt_security_lattice in
    Format.printf "@[Lattice:@. %a@]\n\n" Ifc_pretty.security_lattice lattice);

  let handle_file path info errors =
    match info.FileInfo.file_mode with
    | Some FileInfo.Mstrict ->
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      let { Tast_provider.Compute_tast.tast; _ } =
        Tast_provider.compute_tast_unquarantined ~ctx ~entry
      in
      let check () = Ifc.check opts tast ctx in
      let (new_errors, _) = Errors.do_with_context path check in
      errors @ Errors.get_error_list new_errors
    | _ -> errors
  in

  Relative_path.Map.fold files_info ~init:[] ~f:handle_file
