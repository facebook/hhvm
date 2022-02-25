(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  path: Relative_path.t;
  tast: Tast.program;
  source_text: Full_fidelity_source_text.t option;
}

let create ctx path =
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_unquarantined ~ctx ~entry
  in
  let source_text = entry.Provider_context.source_text in
  { path; tast; source_text }
