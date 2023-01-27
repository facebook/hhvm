(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* TODO[mjt] - this will only remove top-level definititions so,
   for instance, we can't guarantee that `Noop` or `Markup`
   are not contained in say, a function body. We should either
   move these to top-level defs _or_ fully recurse to remove them *)
let on_program program ~ctx =
  let rec aux acc def =
    match def with
    (* These are elaborated away in Namespaces.elaborate_toplevel_defs *)
    | Aast.FileAttributes _
    | Aast.Stmt (_, Aast.Noop)
    | Aast.Stmt (_, Aast.Markup _)
    | Aast.NamespaceUse _
    | Aast.SetNamespaceEnv _ ->
      acc
    | Aast.Stmt _
    | Aast.Fun _
    | Aast.Class _
    | Aast.Typedef _
    | Aast.Constant _
    | Aast.Module _
    | Aast.SetModule _ ->
      def :: acc
    | Aast.Namespace (_ns, aast) -> List.fold_left ~f:aux ~init:[] aast @ acc
  in
  let program = List.rev @@ List.fold_left ~f:aux ~init:[] program in
  (ctx, Ok program)

let pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.{ id with on_ty_program = Some on_program }
