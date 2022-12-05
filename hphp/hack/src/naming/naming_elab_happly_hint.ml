(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Common
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  type t = {
    tparams: SSet.t;
    in_mode: FileInfo.mode;
  }

  let empty = { tparams = SSet.empty; in_mode = FileInfo.Mstrict }

  let add_tparams ps init =
    List.fold
      ps
      ~f:(fun acc Aast.{ tp_name = (_, nm); _ } -> SSet.add nm acc)
      ~init

  let extend_tparams env ps =
    let tparams = add_tparams ps env.tparams in
    { env with tparams }

  let in_class Aast.{ c_mode; c_tparams; _ } =
    { in_mode = c_mode; tparams = add_tparams c_tparams SSet.empty }

  let in_fun_def Aast.{ fd_fun; fd_mode; _ } =
    {
      in_mode = fd_mode;
      tparams = add_tparams fd_fun.Aast.f_tparams SSet.empty;
    }

  let in_typedef Aast.{ t_tparams; t_mode; _ } =
    { in_mode = t_mode; tparams = add_tparams t_tparams SSet.empty }

  let in_gconst Aast.{ cst_mode; _ } =
    { in_mode = cst_mode; tparams = SSet.empty }

  let in_module_def Aast.{ md_mode; _ } =
    { in_mode = md_mode; tparams = SSet.empty }
end

type canon_result =
  | Concrete of Aast.hint
  | This of Pos.t
  | Classname of Pos.t
  | Wildcard of Pos.t
  | Tycon of Pos.t * string
  | Typaram of string
  | Varray of Pos.t
  | Darray of Pos.t
  | Vec_or_dict of Pos.t
  | Error of Naming_error.t

(* A number of hints are represented by `Happly` after lowering; we elaborate
   to the canonical representation here taking care to separate the result
   so we can apply subsequent validation of the hint based on where it appeared *)
let canonical_tycon typarams (pos, name) =
  if String.equal name SN.Typehints.int then
    Concrete (pos, Aast.(Hprim Tint))
  else if String.equal name SN.Typehints.bool then
    Concrete (pos, Aast.(Hprim Tbool))
  else if String.equal name SN.Typehints.float then
    Concrete (pos, Aast.(Hprim Tfloat))
  else if String.equal name SN.Typehints.string then
    Concrete (pos, Aast.(Hprim Tstring))
  else if String.equal name SN.Typehints.darray then
    Darray pos
  else if String.equal name SN.Typehints.varray then
    Varray pos
  (* TODO[mjt] `vec_or_dict` is currently special cased since the canonical representation
     requires us to have no arity mismatches or throw away info. We do not use that repr here
     to avoid having to do so. Ultimately, we should remove that special case *)
  else if
    String.(
      equal name SN.Typehints.varray_or_darray
      || equal name SN.Typehints.vec_or_dict)
  then
    Vec_or_dict pos
  else if String.equal name SN.Typehints.void then
    Concrete (pos, Aast.(Hprim Tvoid))
  else if String.equal name SN.Typehints.noreturn then
    Concrete (pos, Aast.(Hprim Tnoreturn))
  else if String.equal name SN.Typehints.null then
    Concrete (pos, Aast.(Hprim Tnull))
  else if String.equal name SN.Typehints.num then
    Concrete (pos, Aast.(Hprim Tnum))
  else if String.equal name SN.Typehints.resource then
    Concrete (pos, Aast.(Hprim Tresource))
  else if String.equal name SN.Typehints.arraykey then
    Concrete (pos, Aast.(Hprim Tarraykey))
  else if String.equal name SN.Typehints.mixed then
    Concrete (pos, Aast.Hmixed)
  else if String.equal name SN.Typehints.nonnull then
    Concrete (pos, Aast.Hnonnull)
  else if String.equal name SN.Typehints.nothing then
    Concrete (pos, Aast.Hnothing)
  else if String.equal name SN.Typehints.dynamic then
    Concrete (pos, Aast.Hdynamic)
  else if String.equal name SN.Typehints.this then
    This pos
  else if String.equal name SN.Typehints.wildcard then
    Wildcard pos
  else if
    String.(
      equal name ("\\" ^ SN.Typehints.void)
      || equal name ("\\" ^ SN.Typehints.null)
      || equal name ("\\" ^ SN.Typehints.noreturn)
      || equal name ("\\" ^ SN.Typehints.int)
      || equal name ("\\" ^ SN.Typehints.bool)
      || equal name ("\\" ^ SN.Typehints.float)
      || equal name ("\\" ^ SN.Typehints.num)
      || equal name ("\\" ^ SN.Typehints.string)
      || equal name ("\\" ^ SN.Typehints.resource)
      || equal name ("\\" ^ SN.Typehints.mixed)
      || equal name ("\\" ^ SN.Typehints.nonnull)
      || equal name ("\\" ^ SN.Typehints.arraykey)
      || equal name ("\\" ^ SN.Typehints.nothing))
  then
    Error (Naming_error.Primitive_top_level pos)
  (* TODO[mjt] why wouldn't be have a fully qualified name here? *)
  else if String.(equal name SN.Classes.cClassname || equal name "classname")
  then
    Classname pos
  else if SSet.mem name typarams then
    Typaram name
  else
    Tycon (pos, name)

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    method! on_typedef _ t = super#on_typedef Env.(in_typedef t) t

    method! on_gconst _ cst = super#on_gconst Env.(in_gconst cst) cst

    method! on_fun_def _ fd = super#on_fun_def Env.(in_fun_def fd) fd

    method! on_module_def _ md = super#on_module_def Env.(in_module_def md) md

    method! on_class_ _ c = super#on_class_ Env.(in_class c) c

    method! on_method_ env m =
      let env = Env.extend_tparams env m.Aast.m_tparams in
      super#on_method_ env m

    method! on_tparam env tp =
      (* TODO[mjt] do we want to maintain the HKT code? *)
      let env = Env.extend_tparams env tp.Aast.tp_parameters in
      super#on_tparam env tp

    method! on_hint env hint =
      let (hint, canon_err) =
        match hint with
        | (hint_pos, Aast.Happly (tycon, hints)) ->
          (* After lowering many hints are represented as `Happly(...,...)`. Here
             we canonicalise the representation of type constructor then handle
             errors and further elaboration *)
          begin
            match canonical_tycon env.Env.tparams tycon with
            (* The hint was malformed *)
            | Error err -> ((hint_pos, Aast.Herr), Err.naming err)
            (* The type constructors canonical representation is a concrete type *)
            | Concrete (pos, hint_) ->
              (* We can't represent a concrete type applied to other types
                 so we raise an error here *)
              let err =
                if not @@ List.is_empty hints then
                  Err.naming @@ Naming_error.Unexpected_type_arguments pos
                else
                  self#zero
              in
              ((hint_pos, hint_), err)
            (* The type constructors corresponds to an in-scope type parameter *)
            | Typaram name ->
              let hint_ = Aast.Habstr (name, hints) in
              ((hint_pos, hint_), self#zero)
            (* The type constructors canonical representation is `Happly` but
               additional elaboration / validation is required *)
            | This pos ->
              let err =
                if not @@ List.is_empty hints then
                  Err.naming @@ Naming_error.This_no_argument hint_pos
                else
                  self#zero
              in
              ((pos, Aast.Hthis), err)
            | Wildcard pos ->
              if not (List.is_empty hints) then
                let err =
                  Err.naming
                  @@ Naming_error.Tparam_applied_to_type
                       { pos = hint_pos; tparam_name = SN.Typehints.wildcard }
                in
                ((hint_pos, Aast.Herr), err)
              else
                ( (hint_pos, Aast.Happly ((pos, SN.Typehints.wildcard), [])),
                  self#zero )
            | Classname pos ->
              (* TODO[mjt] currently if `classname` is not applied to exactly
                 one type parameter, it canonicalizes to `Hprim Tstring`.
                 Investigate why this happens and if we can delay treatment to
                 typing *)
              (match hints with
              | [_] ->
                let hint_ = Aast.Happly ((pos, SN.Classes.cClassname), hints) in
                ((hint_pos, hint_), self#zero)
              | _ ->
                ( (hint_pos, Aast.(Hprim Tstring)),
                  Err.naming @@ Naming_error.Classname_param pos ))
            | Darray pos -> self#canonicalise_darray env hint_pos pos hints
            | Varray pos -> self#canonicalise_varray env hint_pos pos hints
            | Vec_or_dict pos ->
              self#canonicalise_vec_or_dict env hint_pos pos hints
            (* The type constructors canonical representation is `Happly` *)
            | Tycon (pos, tycon) ->
              let hint_ = Aast.Happly ((pos, tycon), hints) in
              ((hint_pos, hint_), self#zero)
          end
        | _ -> (hint, self#zero)
      in
      let (hint, super_err) = super#on_hint env hint in
      (hint, self#plus canon_err super_err)

    (* TODO[mjt] should we really be special casing `darray`? *)
    method private canonicalise_darray env hint_pos pos hints =
      match hints with
      | [] ->
        let err =
          if not @@ FileInfo.is_hhi env.Env.in_mode then
            Err.naming @@ Naming_error.Too_few_type_arguments hint_pos
          else
            self#zero
        in
        let any = (pos, Aast.Hany) in
        ((hint_pos, Aast.Happly ((pos, SN.Collections.cDict), [any; any])), err)
      | [_] ->
        let err =
          if not @@ FileInfo.is_hhi env.Env.in_mode then
            Err.naming @@ Naming_error.Too_few_type_arguments hint_pos
          else
            self#zero
        in
        ((hint_pos, Aast.Hany), err)
      | [key_hint; val_hint] ->
        ( ( hint_pos,
            Aast.Happly ((pos, SN.Collections.cDict), [key_hint; val_hint]) ),
          self#zero )
      | _ ->
        let err = Err.naming @@ Naming_error.Too_many_type_arguments hint_pos in
        ((hint_pos, Aast.Hany), err)

    (* TODO[mjt] should we really be special casing `varray`? *)
    method private canonicalise_varray env hint_pos pos hints =
      match hints with
      | [] ->
        let err =
          if not @@ FileInfo.is_hhi env.Env.in_mode then
            Err.naming @@ Naming_error.Too_few_type_arguments hint_pos
          else
            self#zero
        in
        let any = (pos, Aast.Hany) in
        ((hint_pos, Aast.Happly ((pos, SN.Collections.cVec), [any])), err)
      | [val_hint] ->
        ( (hint_pos, Aast.Happly ((pos, SN.Collections.cVec), [val_hint])),
          self#zero )
      | _ ->
        let err = Err.naming @@ Naming_error.Too_many_type_arguments hint_pos in
        ((hint_pos, Aast.Hany), err)

    (* TODO[mjt] should we really be special casing `vec_or_dict` both in
       its representation and error handling? *)
    method private canonicalise_vec_or_dict env hint_pos pos hints =
      match hints with
      | [] ->
        let err =
          if not @@ FileInfo.is_hhi env.Env.in_mode then
            Err.naming @@ Naming_error.Too_few_type_arguments hint_pos
          else
            self#zero
        in
        let any = (pos, Aast.Hany) in
        ((hint_pos, Aast.Hvec_or_dict (None, any)), err)
      | [val_hint] -> ((hint_pos, Aast.Hvec_or_dict (None, val_hint)), self#zero)
      | [key_hint; val_hint] ->
        ((hint_pos, Aast.Hvec_or_dict (Some key_hint, val_hint)), self#zero)
      | _ ->
        let err = Err.naming @@ Naming_error.Too_many_type_arguments hint_pos in
        ((hint_pos, Aast.Hany), err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
