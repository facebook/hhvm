(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
include Ast_defs

type nsenv  = Namespace_env.env
type fimode = FileInfo.mode

class ['self] map_base = object (self : 'self)
  inherit [_] Ast_defs.map_defs
  method private on_env          : 'env -> nsenv  -> nsenv  = fun _ x -> x
  method private on_mode         : 'env -> fimode -> fimode = fun _ x -> x
end

class ['self] iter_base = object (self : 'self)
  inherit [_] Ast_defs.iter_defs
  method private on_env          : 'env -> nsenv  -> unit = fun _ _ -> ()
  method private on_mode         : 'env -> fimode -> unit = fun _ _ -> ()
end

class ['self] endo_base = object (self : 'self)
  inherit [_] Ast_defs.endo_defs
  method private on_env          : 'env -> nsenv  -> nsenv  = fun _ x -> x
  method private on_mode         : 'env -> fimode -> fimode = fun _ x -> x
end

class virtual ['self] reduce_base = object (self : 'self)
  inherit [_] Ast_defs.reduce_defs
  method private on_env
    : 'env -> nsenv  -> 'acc = fun _ _ -> self#e
  method private on_mode
    : 'env -> fimode -> 'acc = fun _ _ -> self#e
end
