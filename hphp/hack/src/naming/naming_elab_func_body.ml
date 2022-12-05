(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env : sig
  type t

  val empty : t

  val in_mode : t -> FileInfo.mode

  val set_mode : t -> in_mode:FileInfo.mode -> t
end = struct
  type t = { in_mode: FileInfo.mode }

  let empty = { in_mode = FileInfo.Mstrict }

  let in_mode { in_mode } = in_mode

  let set_mode _ ~in_mode = { in_mode }
end

let on_func_body (env, func_body, err) =
  let func_body =
    if FileInfo.is_hhi @@ Env.in_mode env then
      Aast.{ fb_ast = [] }
    else
      func_body
  in
  Naming_phase_pass.Cont.next (env, func_body, err)

let on_class_ (env, c, err) =
  Naming_phase_pass.Cont.next (Env.set_mode env ~in_mode:c.Aast.c_mode, c, err)

let on_typedef (env, t, err) =
  Naming_phase_pass.Cont.next (Env.set_mode env ~in_mode:t.Aast.t_mode, t, err)

let on_gconst (env, cst, err) =
  Naming_phase_pass.Cont.next
    (Env.set_mode env ~in_mode:cst.Aast.cst_mode, cst, err)

let on_fun_def (env, fd, err) =
  Naming_phase_pass.Cont.next
    (Env.set_mode env ~in_mode:fd.Aast.fd_mode, fd, err)

let on_module_def (env, md, err) =
  Naming_phase_pass.Cont.next
    (Env.set_mode env ~in_mode:md.Aast.md_mode, md, err)

let pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_func_body = Some on_func_body;
        on_class_ = Some on_class_;
        on_typedef = Some on_typedef;
        on_gconst = Some on_gconst;
        on_fun_def = Some on_fun_def;
        on_module_def = Some on_module_def;
      })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?(env = Env.empty) elem = fst @@ f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
