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
      Naming_phase_env.t * ('a, 'b) Aast_defs.func_body ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.func_body, _) result =
 fun (env, func_body) ->
  let func_body =
    if FileInfo.is_hhi @@ Env.in_mode env then
      Aast.{ fb_ast = [] }
    else
      func_body
  in
  Ok (env, func_body)

let on_class_ :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.class_ ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.class_, _) result =
 (fun (env, c) -> Ok (Env.set_mode env ~in_mode:c.Aast.c_mode, c))

let on_typedef :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.typedef ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.typedef, _) result =
 (fun (env, t) -> Ok (Env.set_mode env ~in_mode:t.Aast.t_mode, t))

let on_gconst :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.gconst ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.gconst, _) result =
 (fun (env, cst) -> Ok (Env.set_mode env ~in_mode:cst.Aast.cst_mode, cst))

let on_fun_def :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.fun_def ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.fun_def, _) result =
 (fun (env, fd) -> Ok (Env.set_mode env ~in_mode:fd.Aast.fd_mode, fd))

let on_module_def :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.module_def ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.module_def, _) result =
 (fun (env, md) -> Ok (Env.set_mode env ~in_mode:md.Aast.md_mode, md))

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
