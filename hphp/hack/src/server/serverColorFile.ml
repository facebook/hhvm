(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let go_quarantined ~(ctx : Provider_context.t) ~(entry : Provider_context.entry)
    =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  match Coverage_level.get_levels ctx tast entry.Provider_context.path with
  | Ok levels -> levels
  | Error () ->
    (* The "Fixme Provider" will return an error if the file cannot be found.
    Let's convert that error to a plain result. *)
    ( [],
      {
        Coverage_level_defs.checked = 0;
        Coverage_level_defs.partial = 0;
        Coverage_level_defs.unchecked = 0;
      } )
