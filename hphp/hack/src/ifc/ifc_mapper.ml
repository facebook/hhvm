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

let rec ptype fty fpol = function
  | Tprim pol -> Tprim (fpol pol)
  | Tgeneric pol -> Tgeneric (fpol pol)
  | Ttuple tl -> Ttuple (List.map ~f:fty tl)
  | Tunion tl -> Tunion (List.map ~f:fty tl)
  | Tinter tl -> Tinter (List.map ~f:fty tl)
  | Tclass cls ->
    Tclass
      {
        c_name = cls.c_name;
        c_self = fpol cls.c_self;
        c_lump = fpol cls.c_lump;
      }
  | Tfun f -> Tfun (fun_ fty fpol f)
  | Tcow_array { a_kind; a_key; a_value; a_length } ->
    Tcow_array
      {
        a_kind;
        a_key = fty a_key;
        a_value = fty a_value;
        a_length = fpol a_length;
      }

and fun_ fty fpol f =
  let ptype = ptype fty fpol in
  {
    f_pc = fpol f.f_pc;
    f_self = fpol f.f_self;
    f_args = List.map ~f:ptype f.f_args;
    f_ret = ptype f.f_ret;
    f_exn = ptype f.f_exn;
  }

let iter_ptype2 fty fpol pt1 pt2 =
  let flist l1 l2 =
    match List.iter2 ~f:fty l1 l2 with
    | List.Or_unequal_lengths.Ok () -> ()
    | _ -> invalid_arg "iter_ptype2"
  in
  match (pt1, pt2) with
  | (Tprim p1, Tprim p2)
  | (Tgeneric p1, Tgeneric p2) ->
    fpol p1 p2
  | (Ttuple tl1, Ttuple tl2)
  | (Tunion tl1, Tunion tl2)
  | (Tinter tl1, Tinter tl2) ->
    flist tl1 tl2
  | (Tclass cls1, Tclass cls2) ->
    (* ignore property map, it is going away soon *)
    fpol cls1.c_self cls2.c_self;
    fpol cls1.c_lump cls2.c_lump
  | (Tfun f1, Tfun f2) ->
    fpol f1.f_pc f2.f_pc;
    fpol f1.f_self f2.f_self;
    flist f1.f_args f2.f_args;
    fty f1.f_ret f2.f_ret;
    fty f1.f_exn f2.f_exn
  | _ -> invalid_arg "iter_ptype2"

(* "fprop: int -> prop -> prop" takes as first argument the
   number of binders under which the prop argument is; it is
   initialized by the "depth" argument *)
let prop fpol fprop depth = function
  | Ctrue -> Ctrue
  | Cquant (q, n, c) -> Cquant (q, n, fprop (depth + n) c)
  | Ccond ((pos, p, x), ct, ce) ->
    Ccond ((pos, fpol p, x), fprop depth ct, fprop depth ce)
  | Cconj (cl, cr) -> Cconj (fprop depth cl, fprop depth cr)
  | Cflow (pos, p1, p2) -> Cflow (pos, fpol p1, fpol p2)
  | Chole (pos, proto) ->
    if phys_equal fpol Ifc_utils.identity then
      Chole (pos, proto)
    else
      (* "pty_map pty" applies fpol to all the policies in the
         flow type pty *)
      let rec pty_map pty = ptype pty_map fpol pty in
      let proto =
        {
          fp_name = proto.fp_name;
          fp_this = Option.map ~f:pty_map proto.fp_this;
          fp_type = fun_ pty_map fpol proto.fp_type;
        }
      in
      Chole (pos, proto)
