(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SM = Ast_defs.ShapeMap
module LM = Local_id.Map

class virtual ['self] iter =
  object (_ : 'self)
    inherit [_] Ast_defs.iter

    method private on_local_id_map
        : 'a. ('env -> 'a -> unit) -> 'env -> 'a LM.t -> unit =
      (fun f env -> LM.iter (fun _ -> f env))

    method on_'ex _ _ = ()

    method on_'en _ _ = ()
  end

class virtual ['self] reduce =
  object (self : 'self)
    inherit [_] Ast_defs.reduce

    method private on_local_id_map
        : 'a. ('env -> 'a -> 'acc) -> 'env -> 'a LM.t -> 'acc =
      fun f env x ->
        LM.fold (fun _ d acc -> self#plus acc (f env d)) x self#zero

    method on_'ex _env _ = self#zero

    method on_'en _env _ = self#zero
  end

class virtual ['self] map =
  object (_ : 'self)
    inherit [_] Ast_defs.map

    method private on_local_id_map
        : 'a 'b. ('env -> 'a -> 'b) -> 'env -> 'a LM.t -> 'b LM.t =
      (fun f env -> LM.map (f env))
  end

class virtual ['self] endo =
  object (_ : 'self)
    inherit [_] Ast_defs.endo

    method private on_local_id_map
        : 'a 'b. ('env -> 'a -> 'b) -> 'env -> 'a LM.t -> 'b LM.t =
      (fun f env -> LM.map (f env))
  end

class virtual ['self] mapreduce =
  object (self : 'self)
    inherit [_] Ast_defs.mapreduce

    method private on_local_id_map
        : 'a 'b. ('env -> 'a -> 'b * 'acc) -> 'env -> 'a LM.t -> 'b LM.t * 'acc
        =
      fun f env x ->
        let (err, x) =
          LM.map_env
            (fun err _k d ->
              let (x, e) = f env d in
              (self#plus err e, x))
            self#zero
            x
        in
        (x, err)

    method on_'ex _env x = (x, self#zero)

    method on_'en _env x = (x, self#zero)
  end
