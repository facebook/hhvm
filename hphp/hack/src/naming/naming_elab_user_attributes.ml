(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env = struct
  type t = unit

  let empty = ()
end

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    method private check_user_attrs env us =
      let seen = Caml.Hashtbl.create 0 in
      let dedup (attrs, err) (Aast.{ ua_name = (pos, attr_name); _ } as attr) =
        match Caml.Hashtbl.find_opt seen attr_name with
        | Some prev_pos ->
          ( attrs,
            self#plus err
            @@ Err.naming
            @@ Naming_error.Duplicate_user_attribute
                 { pos; prev_pos; attr_name } )
        | _ ->
          Caml.Hashtbl.add seen attr_name pos;
          let (attr, attr_err) = super#on_user_attribute env attr in
          (attr :: attrs, self#plus err attr_err)
      in
      Tuple2.map_fst ~f:List.rev
      @@ List.fold_left us ~init:([], self#zero) ~f:dedup

    method! on_fun_param env fp =
      let (param_user_attributes, err) =
        self#check_user_attrs env fp.Aast.param_user_attributes
      in
      let (fp, super_err) =
        super#on_fun_param env Aast.{ fp with param_user_attributes }
      in
      (fp, self#plus err super_err)

    method! on_fun_ env fn =
      let (f_user_attributes, err) =
        self#check_user_attrs env fn.Aast.f_user_attributes
      in
      let (fn, super_err) =
        super#on_fun_ env Aast.{ fn with f_user_attributes }
      in
      (fn, self#plus err super_err)

    method! on_file_attribute env fa =
      let (fa_user_attributes, err) =
        self#check_user_attrs env fa.Aast.fa_user_attributes
      in
      let (fa, super_err) =
        super#on_file_attribute env Aast.{ fa with fa_user_attributes }
      in
      (fa, self#plus err super_err)

    method! on_tparam env tp =
      let (tp_user_attributes, err) =
        self#check_user_attrs env tp.Aast.tp_user_attributes
      in
      let (tp, super_err) =
        super#on_tparam env Aast.{ tp with tp_user_attributes }
      in
      (tp, self#plus err super_err)

    method! on_class_ env c =
      let (c_user_attributes, err) =
        self#check_user_attrs env c.Aast.c_user_attributes
      in
      let (c, super_err) =
        super#on_class_ env Aast.{ c with c_user_attributes }
      in
      (c, self#plus err super_err)

    method! on_class_const env cc =
      let (cc_user_attributes, err) =
        self#check_user_attrs env cc.Aast.cc_user_attributes
      in
      let (cc, super_err) =
        super#on_class_const env Aast.{ cc with cc_user_attributes }
      in
      (cc, self#plus err super_err)

    method! on_class_typeconst_def env c =
      let (c_tconst_user_attributes, err) =
        self#check_user_attrs env c.Aast.c_tconst_user_attributes
      in
      let (c, super_err) =
        super#on_class_typeconst_def
          env
          Aast.{ c with c_tconst_user_attributes }
      in
      (c, self#plus err super_err)

    method! on_class_var env cv =
      let (cv_user_attributes, err) =
        self#check_user_attrs env cv.Aast.cv_user_attributes
      in
      let (cv, super_err) =
        super#on_class_var env Aast.{ cv with cv_user_attributes }
      in
      (cv, self#plus err super_err)

    method! on_method_ env m =
      let (m_user_attributes, err) =
        self#check_user_attrs env m.Aast.m_user_attributes
      in
      let (m, super_err) =
        super#on_method_ env Aast.{ m with m_user_attributes }
      in
      (m, self#plus err super_err)

    method! on_typedef env t =
      let (t_user_attributes, err) =
        self#check_user_attrs env t.Aast.t_user_attributes
      in
      let (t, super_err) =
        super#on_typedef env Aast.{ t with t_user_attributes }
      in
      (t, self#plus err super_err)

    method! on_module_def env md =
      let (md_user_attributes, err) =
        self#check_user_attrs env md.Aast.md_user_attributes
      in
      let (md, super_err) =
        super#on_module_def env Aast.{ md with md_user_attributes }
      in
      (md, self#plus err super_err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
