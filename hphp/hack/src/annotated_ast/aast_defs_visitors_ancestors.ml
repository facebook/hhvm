(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SM = Ast.ShapeMap

class ['self] iter_defs_base = object (self : 'self)
  inherit [_] Ast.iter_defs
  method private on_shape_map
    : 'a . ('env -> 'a -> unit) -> 'env -> 'a SM.t -> unit
    = fun f env x -> SM.iter (self#on_shape_map_entry f env) x
  method private on_shape_map_entry
    : 'a . ('env -> 'a -> unit) -> 'env -> SM.key -> 'a -> unit
    = fun f env key data ->
      self#on_shape_field_name env key;
      f env data
end

class virtual ['self] reduce_defs_base = object (self : 'self)
  inherit [_] Ast.reduce_defs
  method private on_shape_map
    : 'a . ('env -> 'a -> 'acc) -> 'env -> 'a SM.t -> 'acc
    = fun f env x ->
      SM.fold (fun k d acc -> self#plus acc (self#on_shape_map_entry f env k d))
        x self#zero
  method private on_shape_map_entry
    : 'a . ('env -> 'a -> 'acc) -> 'env -> SM.key -> 'a -> 'acc
    = fun f env key data ->
      self#plus (self#on_shape_field_name env key) (f env data)
end

class ['self] map_defs_base = object (self : 'self)
  inherit [_] Ast.map_defs
  method private on_shape_map
    : 'a 'b . ('env -> 'a -> 'b) -> 'env -> 'a SM.t -> 'b SM.t
    = fun f env x ->
      let map_entry key data acc =
        let key = self#on_shape_field_name env key in
        let data = f env data in
        SM.add key data acc
      in
      SM.fold map_entry x SM.empty
end

class ['self] endo_defs_base = object (self : 'self)
  inherit [_] Ast.endo_defs
  method private on_shape_map
    : 'a 'b . ('env -> 'a -> 'b) -> 'env -> 'a SM.t -> 'b SM.t
    = fun f env x ->
      (* FIXME: Should be possible to write a true (more efficient) endo
         implementation rather than copying map *)
      let map_entry key data acc =
        let key = self#on_shape_field_name env key in
        let data = f env data in
        SM.add key data acc
      in
      SM.fold map_entry x SM.empty
end
