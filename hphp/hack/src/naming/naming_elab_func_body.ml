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

module Env = struct
  type t = FileInfo.mode

  let empty = FileInfo.Mstrict
end

let visitor =
  object (_self)
    inherit [_] Aast_defs.endo as super

    method on_'ex _ ex = ex

    method on_'en _ en = en

    method! on_func_body mode func_body =
      let func_body =
        if FileInfo.is_hhi mode then
          Aast.{ fb_ast = [] }
        else
          func_body
      in
      super#on_func_body mode func_body

    method! on_class_ _ c = super#on_class_ c.Aast.c_mode c

    method! on_typedef _ t = super#on_typedef t.Aast.t_mode t

    method! on_gconst _ cst = super#on_gconst cst.Aast.cst_mode cst

    method! on_fun_def _ fd = super#on_fun_def fd.Aast.fd_mode fd

    method! on_module_def _ md = super#on_module_def md.Aast.md_mode md
  end

let elab f ?(env = Env.empty) elem = f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
