(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

class virtual ['self] reduce = object (self : 'self)
  inherit [_] Tast.reduce as super

  val mutable saved_env = None;

  method private saved_env =
    match saved_env with
    | Some saved_env -> saved_env
    | None -> failwith "No containing node had an env annotation"

  method private with_env env f =
    let prev_env = saved_env in
    saved_env <- Some env;
    let result = f () in
    saved_env <- prev_env;
    result

  method! on_fun_ acc x =
    self#with_env x.Tast.f_annotation (fun () -> super#on_fun_ acc x)

  method! on_class_ acc x =
    self#with_env x.Tast.c_annotation (fun () -> super#on_class_ acc x)

  method! on_method_ acc x =
    self#with_env x.Tast.m_annotation (fun () -> super#on_method_ acc x)

  method! on_typedef acc x =
    self#with_env x.Tast.t_annotation (fun () -> super#on_typedef acc x)

  method! on_gconst acc x =
    self#with_env x.Tast.cst_annotation (fun () -> super#on_gconst acc x)
end
