(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ifc_types

(* Shallow mappers *)

let rec ptype fty fpol = function
  | Tnull pol -> Tnull (fpol pol)
  | Tprim pol -> Tprim (fpol pol)
  | Tnonnull (pself, plump) -> Tnonnull (fpol pself, fpol plump)
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
  | Tshape { sh_kind; sh_fields } ->
    let sh_kind =
      match sh_kind with
      | Open_shape ty -> Open_shape (fty ty)
      | Closed_shape -> Closed_shape
    in
    let field { sft_optional; sft_policy; sft_ty } =
      { sft_optional; sft_policy = fpol sft_policy; sft_ty = fty sft_ty }
    in
    Tshape { sh_kind; sh_fields = Typing_defs.TShapeMap.map field sh_fields }
  | Tdynamic pol -> Tdynamic (fpol pol)

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
  | (Tnull p1, Tnull p2)
  | (Tprim p1, Tprim p2)
  | (Tgeneric p1, Tgeneric p2)
  | (Tdynamic p1, Tdynamic p2) ->
    fpol p1 p2
  | (Tnonnull (ps1, pl1), Tnonnull (ps2, pl2)) ->
    fpol ps1 ps2;
    fpol pl1 pl2
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
  | (Tcow_array a1, Tcow_array a2) ->
    (* Assume the array kinds are ok *)
    fty a1.a_key a2.a_key;
    fty a1.a_value a2.a_value;
    fpol a1.a_length a2.a_length
  | (Tshape s1, Tshape s2) ->
    let combine _ f1 f2 =
      match (f1, f2) with
      | (Some t1, Some t2) ->
        fpol t1.sft_policy t2.sft_policy;
        fty t1.sft_ty t2.sft_ty;
        None
      | _ -> invalid_arg "iter_ptype2"
    in
    begin
      match (s1.sh_kind, s2.sh_kind) with
      | (Closed_shape, Closed_shape) -> ()
      | (Open_shape t1, Open_shape t2) -> fty t1 t2
      | (_, _) -> invalid_arg "iter_ptype2"
    end;
    ignore (Typing_defs.TShapeMap.merge combine s1.sh_fields s2.sh_fields)
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
