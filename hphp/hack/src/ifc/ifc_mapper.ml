(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Ifc_types

(* Shallow mappers *)

let ptype fty fpol = function
  | Tprim pol -> Tprim (fpol pol)
  | Ttuple tl -> Ttuple (List.map ~f:fty tl)
  | Tunion tl -> Tunion (List.map ~f:fty tl)
  | Tinter tl -> Tinter (List.map ~f:fty tl)
  | Tclass cls ->
    let prop_map =
      SMap.map (fun lpty -> lazy (fty (Lazy.force lpty))) cls.c_property_map
    in
    Tclass
      {
        c_name = cls.c_name;
        c_self = fpol cls.c_self;
        c_lump = fpol cls.c_lump;
        c_property_map = prop_map;
        c_tparams = List.map ~f:(fun (pty, var) -> (fty pty, var)) cls.c_tparams;
      }

(* "fprop: int -> prop -> prop" takes as first argument the
   number of binders under which the prop argument is; it is
   initialized by the "depth" argument *)
let prop fpol fprop depth = function
  | Ctrue -> Ctrue
  | Cquant (q, n, c) -> Cquant (q, n, fprop (depth + n) c)
  | Ccond ((p, x), ct, ce) -> Ccond ((fpol p, x), fprop depth ct, fprop depth ce)
  | Cconj (cl, cr) -> Cconj (fprop depth cl, fprop depth cr)
  | Cflow (p1, p2) -> Cflow (fpol p1, fpol p2)
  | Chole proto ->
    if phys_equal fpol Ifc_utils.identity then
      Chole proto
    else
      (* "pty_map pty" applies fpol to all the policies in the
         flow type pty *)
      let rec pty_map pty = ptype pty_map fpol pty in
      Chole
        {
          fp_name = proto.fp_name;
          fp_pc = fpol proto.fp_pc;
          fp_this = Option.map ~f:pty_map proto.fp_this;
          fp_args = List.map ~f:pty_map proto.fp_args;
          fp_ret = pty_map proto.fp_ret;
        }
