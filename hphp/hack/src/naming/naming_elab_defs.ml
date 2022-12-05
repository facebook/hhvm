(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Env : sig
  type t

  val empty : t
end = struct
  type t = unit

  let empty = ()
end

(* TODO[mjt] - this will only remove top-level definititions so,
   for instance, we can't guarantee that `Noop` or `Markup`
   are not contained in say, a function body. We should either
   move these to top-level defs _or_ fully recurse to remove them *)
let on_program (env, program, err) =
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
  Naming_phase_pass.Cont.finish (env, program, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_program = Some on_program })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?(env = Env.empty) elem = fst @@ f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
