(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

class virtual ['e] monoid =
  object
    method virtual private zero : 'e

    method virtual private plus : 'e -> 'e -> 'e
  end

class virtual ['a] option_monoid =
  object (self)
    inherit ['a option] monoid

    method virtual private merge : 'a -> 'a -> 'a

    method private zero = None

    method private plus = Option.merge ~f:self#merge
  end

class ['self] map_defs_base =
  object
    method private on_string : 'env -> string -> string = (fun _ x -> x)

    method private on_int : 'env -> int -> int = (fun _ x -> x)

    method private on_bool : 'env -> bool -> bool = (fun _ x -> x)

    method private on_list
        : 'env 'a 'b. ('env -> 'a -> 'b) -> 'env -> 'a list -> 'b list =
      (fun f env -> List.map ~f:(f env))

    method private on_option
        : 'env 'a 'b. ('env -> 'a -> 'b) -> 'env -> 'a option -> 'b option =
      (fun f env -> Option.map ~f:(f env))
  end

class ['self] iter_defs_base =
  object
    method private on_string : 'env -> string -> unit = (fun _ _ -> ())

    method private on_int : 'env -> int -> unit = (fun _ _ -> ())

    method private on_bool : 'env -> bool -> unit = (fun _ _ -> ())

    method private on_list
        : 'env 'a. ('env -> 'a -> unit) -> 'env -> 'a list -> unit =
      (fun f env -> List.iter ~f:(f env))

    method private on_option
        : 'env 'a. ('env -> 'a -> unit) -> 'env -> 'a option -> unit =
      (fun f env -> Option.iter ~f:(f env))
  end

class ['self] endo_defs_base =
  object
    method private on_string : 'env -> string -> string = (fun _ x -> x)

    method private on_int : 'env -> int -> int = (fun _ x -> x)

    method private on_bool : 'env -> bool -> bool = (fun _ x -> x)

    method private on_list
        : 'env 'a. ('env -> 'a -> 'a) -> 'env -> 'a list -> 'a list =
      fun f env xs ->
        let rec aux env xs counter =
          match xs with
          | [] -> xs
          | [y1] ->
            let z1 = f env y1 in
            if phys_equal y1 z1 then
              xs
            else
              [z1]
          | [y1; y2] ->
            let z1 = f env y1 in
            let z2 = f env y2 in
            if phys_equal y1 z1 && phys_equal y2 z2 then
              xs
            else
              [z1; z2]
          | [y1; y2; y3] ->
            let z1 = f env y1 in
            let z2 = f env y2 in
            let z3 = f env y3 in
            if phys_equal y1 z1 && phys_equal y2 z2 && phys_equal y3 z3 then
              xs
            else
              [z1; z2; z3]
          | [y1; y2; y3; y4] ->
            let z1 = f env y1 in
            let z2 = f env y2 in
            let z3 = f env y3 in
            let z4 = f env y4 in
            if
              phys_equal y1 z1
              && phys_equal y2 z2
              && phys_equal y3 z3
              && phys_equal y4 z4
            then
              xs
            else
              [z1; z2; z3; z4]
          | [y1; y2; y3; y4; y5] ->
            let z1 = f env y1 in
            let z2 = f env y2 in
            let z3 = f env y3 in
            let z4 = f env y4 in
            let z5 = f env y5 in
            if
              phys_equal y1 z1
              && phys_equal y2 z2
              && phys_equal y3 z3
              && phys_equal y4 z4
              && phys_equal y5 z5
            then
              xs
            else
              [z1; z2; z3; z4; z5]
          | y1 :: y2 :: y3 :: y4 :: y5 :: ys ->
            let z1 = f env y1 in
            let z2 = f env y2 in
            let z3 = f env y3 in
            let z4 = f env y4 in
            let z5 = f env y5 in
            let zs =
              if counter > 1000 then
                aux_slow env ys [] ys false
              else
                aux env ys (counter + 1)
            in
            if
              phys_equal y1 z1
              && phys_equal y2 z2
              && phys_equal y3 z3
              && phys_equal y4 z4
              && phys_equal y5 z5
              && phys_equal ys zs
            then
              xs
            else
              z1 :: z2 :: z3 :: z4 :: z5 :: zs
        and aux_slow env xs acc original_list has_new_elements =
          match xs with
          | [] ->
            if has_new_elements then
              List.rev acc
            else
              original_list
          | y1 :: ys ->
            let z1 = f env y1 in
            aux_slow
              env
              ys
              (z1 :: acc)
              original_list
              (has_new_elements || not (phys_equal y1 z1))
        in
        aux env xs 0

    method private on_option
        : 'env 'a. ('env -> 'a -> 'a) -> 'env -> 'a option -> 'a option =
      fun f env x ->
        match x with
        | None -> x
        | Some y ->
          let z = f env y in
          if phys_equal y z then
            x
          else
            Some z
  end

class virtual ['self] reduce_defs_base =
  object (self : 'self)
    inherit ['acc] monoid

    method private on_string : 'env -> string -> 'acc = (fun _ _ -> self#zero)

    method private on_int : 'env -> int -> 'acc = (fun _ _ -> self#zero)

    method private on_bool : 'env -> bool -> 'acc = (fun _ _ -> self#zero)

    method private on_list
        : 'env 'a. ('env -> 'a -> 'acc) -> 'env -> 'a list -> 'acc =
      (fun f env xs -> self#list_fold_left f env self#zero xs)

    method private list_fold_left
        : 'env 'a. ('env -> 'a -> 'acc) -> 'env -> 'acc -> 'a list -> 'acc =
      fun f env acc xs ->
        match xs with
        | [] -> acc
        | y :: ys ->
          let acc = self#plus acc (f env y) in
          self#list_fold_left f env acc ys

    method private on_option
        : 'env 'a. ('env -> 'a -> 'acc) -> 'env -> 'a option -> 'acc =
      (fun f env -> Option.value_map ~default:self#zero ~f:(f env))
  end
