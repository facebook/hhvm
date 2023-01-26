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

let on_func_body :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.func_body * 'c ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.func_body * 'c, 'd) result =
 fun (env, func_body, err) ->
  let func_body =
    if FileInfo.is_hhi @@ Env.in_mode env then
      Aast.{ fb_ast = [] }
    else
      func_body
  in
  Ok (env, func_body, err)

let on_class_ :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.class_ * 'c ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.class_ * 'c, 'd) result =
 (fun (env, c, err) -> Ok (Env.set_mode env ~in_mode:c.Aast.c_mode, c, err))

let on_typedef :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.typedef * 'c ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.typedef * 'c, 'd) result =
 (fun (env, t, err) -> Ok (Env.set_mode env ~in_mode:t.Aast.t_mode, t, err))

let on_gconst :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.gconst * 'c ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.gconst * 'c, 'd) result =
 fun (env, cst, err) ->
  Ok (Env.set_mode env ~in_mode:cst.Aast.cst_mode, cst, err)

let on_fun_def :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.fun_def * 'c ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.fun_def * 'c, 'd) result =
 (fun (env, fd, err) -> Ok (Env.set_mode env ~in_mode:fd.Aast.fd_mode, fd, err))

let on_module_def :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.module_def * 'c ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.module_def * 'c, 'd) result =
 (fun (env, md, err) -> Ok (Env.set_mode env ~in_mode:md.Aast.md_mode, md, err))

let pass =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        {
          identity with
          on_func_body = Some on_func_body;
          on_class_ = Some on_class_;
          on_typedef = Some on_typedef;
          on_gconst = Some on_gconst;
          on_fun_def = Some on_fun_def;
          on_module_def = Some on_module_def;
        })
