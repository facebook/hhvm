(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

class virtual ['e] monoid = object (self)
  method private virtual e: 'e
  method private virtual add: 'e -> 'e -> 'e
  method private sum: 'e list -> 'e = List.fold_left self#add self#e
end

class ['e] monoid_sum = object
  inherit ['e] monoid
  method private e = 0
  method private add = (+)
end

class ['e] monoid_product = object
  inherit ['e] monoid
  method private e = 1
  method private add = ( * )
end

class ['self] map_defs_base = object (self : 'self)
  method private on_t            : 'env -> Pos.t  -> Pos.t  = fun _ x -> x
  method private on_string       : 'env -> string -> string = fun _ x -> x
  method private on_option
    : 'env 'a 'b . ('env -> 'a -> 'b) -> 'env -> 'a option -> 'b option
    = fun f env -> Option.map ~f:(f env)
end

class ['self] iter_defs_base = object (self : 'self)
  method private on_t            : 'env -> Pos.t  -> unit = fun _ _ -> ()
  method private on_string       : 'env -> string -> unit = fun _ _ -> ()
  method private on_option
    : 'env 'a . ('env -> 'a -> unit) -> 'env -> 'a option -> unit
    = fun f env -> Option.iter ~f:(f env)
end

class ['self] endo_defs_base = object (self : 'self)
  method private on_t            : 'env -> Pos.t  -> Pos.t  = fun _ x -> x
  method private on_string       : 'env -> string -> string = fun _ x -> x
  method private on_option
    : 'env 'a . ('env -> 'a -> 'a) -> 'env -> 'a option -> 'a option
    = fun f env x ->
      match x with
      | None -> x
      | Some y ->
        let z = f env y in
        if y == z then x else Some z
end

class virtual ['self] reduce_defs_base = object (self : 'self)
  inherit ['acc] monoid
  method private on_t
    : 'env -> Pos.t  -> 'acc = fun _ _ -> self#e
  method private on_string
    : 'env -> string -> 'acc = fun _ _ -> self#e

  method private zero : 'acc = self#e

  method private plus : 'a -> 'a -> 'acc = self#add

  method private on_option
    : 'env 'a . ('env -> 'a -> 'acc) -> 'env -> 'a option -> 'acc
    = fun f env -> Option.value_map ~default:self#e ~f:(f env)
end
