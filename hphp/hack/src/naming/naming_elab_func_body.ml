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
  let in_mode
      Naming_phase_env.{ elab_func_body = Elab_func_body.{ in_mode }; _ } =
    in_mode

  let set_mode t ~in_mode =
    Naming_phase_env.{ t with elab_func_body = Elab_func_body.{ in_mode } }
end

let on_func_body func_body ~ctx =
  let func_body =
    if FileInfo.is_hhi @@ Env.in_mode ctx then
      Aast.{ fb_ast = [] }
    else
      func_body
  in
  (ctx, Ok func_body)

let on_class_ c ~ctx = (Env.set_mode ctx ~in_mode:c.Aast.c_mode, Ok c)

let on_typedef t ~ctx = (Env.set_mode ctx ~in_mode:t.Aast.t_mode, Ok t)

let on_gconst cst ~ctx = (Env.set_mode ctx ~in_mode:cst.Aast.cst_mode, Ok cst)

let on_fun_def fd ~ctx = (Env.set_mode ctx ~in_mode:fd.Aast.fd_mode, Ok fd)

let on_module_def md ~ctx = (Env.set_mode ctx ~in_mode:md.Aast.md_mode, Ok md)

let pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_func_body = Some on_func_body;
        on_ty_class_ = Some on_class_;
        on_ty_typedef = Some on_typedef;
        on_ty_gconst = Some on_gconst;
        on_ty_fun_def = Some on_fun_def;
        on_ty_module_def = Some on_module_def;
      }
