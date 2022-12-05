(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  type t = {
    in_enum_class: bool;
    in_mode: FileInfo.mode;
  }

  let empty = { in_enum_class = false; in_mode = FileInfo.Mstrict }

  let create ?(in_enum_class = false) ?(in_mode = FileInfo.Mstrict) () =
    { in_enum_class; in_mode }
end

let visitor =
  object (self)
    inherit [_] Naming_visitors.mapreduce as super

    method! on_class_ _env c =
      let in_enum_class =
        match c.Aast.c_kind with
        | Ast_defs.Cenum_class _ -> true
        | Ast_defs.(Cclass _ | Cinterface | Cenum | Ctrait) -> false
      in
      let env = Env.(create ~in_enum_class ~in_mode:c.Aast.c_mode ()) in
      super#on_class_ env c

    method! on_class_const_kind env kind =
      let (kind, err) =
        match kind with
        | Aast.CCConcrete expr ->
          let (expr, err) = self#const_expr env expr in
          (Aast.CCConcrete expr, err)
        | Aast.CCAbstract _ -> (kind, self#zero)
      in
      let (kind, super_err) = super#on_class_const_kind env kind in
      (kind, self#plus err super_err)

    method! on_gconst env cst =
      let env = Env.{ env with in_mode = cst.Aast.cst_mode } in
      super#on_gconst env cst

    method! on_gconst_cst_value env cst_value =
      let (cst_value, err) = self#const_expr env cst_value in
      let (cst_value, super_err) = super#on_gconst_cst_value env cst_value in
      (cst_value, self#plus err super_err)

    method! on_typedef env t =
      super#on_typedef Env.{ env with in_mode = t.Aast.t_mode } t

    method! on_fun_def env fd =
      super#on_fun_def Env.{ env with in_mode = fd.Aast.fd_mode } fd

    method! on_module_def env md =
      super#on_module_def Env.{ env with in_mode = md.Aast.md_mode } md

    method private const_expr env ((_, pos, _) as expr) =
      if env.Env.in_enum_class then
        (expr, self#zero)
      else
        let (is_const_expr, err) = self#is_const_expr env expr in
        if is_const_expr then
          (expr, err)
        else
          (((), pos, Err.invalid_expr_ pos), err)

    method private is_const_expr env (_, pos, expr_) =
      match expr_ with
      | Aast.(Id _ | Null | True | False | Int _ | Float _ | String _) ->
        (true, self#zero)
      | Aast.(Class_const ((_, _, (CIparent | CIself | CI _)), _)) ->
        (true, self#zero)
      | Aast.(Class_const ((_, _, Aast.CIexpr (_, _, (This | Id _))), _)) ->
        (true, self#zero)
      | Aast.(Smethod_id _ | Fun_id _) -> (true, self#zero)
      | Aast.(FunctionPointer ((FP_id _ | FP_class_const _), _)) ->
        (true, self#zero)
      | Aast.Upcast (e, _) -> self#is_const_expr env e
      | Aast.(As (e, (_, Hlike _), _)) -> self#is_const_expr env e
      | Aast.(As (e, (_, Happly ((p, cn), [_])), _)) ->
        if String.equal cn SN.FB.cIncorrectType then
          self#is_const_expr env e
        else
          (false, Err.naming @@ Naming_error.Illegal_constant p)
      | Aast.Unop
          ( (Ast_defs.Uplus | Ast_defs.Uminus | Ast_defs.Utild | Ast_defs.Unot),
            e ) ->
        self#is_const_expr env e
      | Aast.Binop (op, e1, e2) ->
        (* Only assignment is invalid *)
        begin
          match op with
          | Ast_defs.Eq _ ->
            let err = Err.naming @@ Naming_error.Illegal_constant pos in
            (false, err)
          | _ ->
            self#and_ (self#is_const_expr env e1) (self#is_const_expr env e2)
        end
      | Aast.Eif (e1, e2_opt, e3) ->
        self#and_
          (self#and_
             (self#is_const_expr env e1)
             (Option.value_map
                ~default:(true, self#zero)
                ~f:(self#is_const_expr env)
                e2_opt))
          (self#is_const_expr env e3)
      | Aast.Darray (_, kvs)
      | Aast.(KeyValCollection (Dict, _, kvs)) ->
        List.fold_left kvs ~init:(true, self#zero) ~f:(fun acc (ek, ev) ->
            self#and_ acc
            @@ self#and_ (self#is_const_expr env ek) (self#is_const_expr env ev))
      | Aast.Varray (_, exprs)
      | Aast.(ValCollection ((Vec | Keyset), _, exprs))
      | Aast.Tuple exprs ->
        List.fold_left exprs ~init:(true, self#zero) ~f:(fun acc e ->
            self#and_ acc @@ self#is_const_expr env e)
      | Aast.Shape flds ->
        List.fold_left flds ~init:(true, self#zero) ~f:(fun acc (_, e) ->
            self#and_ acc @@ self#is_const_expr env e)
      | Aast.Call ((_, _, Aast.Id (_, cn)), _, fn_params, _)
        when String.equal cn SN.StdlibFunctions.array_mark_legacy
             || String.equal cn SN.PseudoFunctions.unsafe_cast
             || String.equal cn SN.PseudoFunctions.unsafe_nonnull_cast ->
        List.fold_left fn_params ~init:(true, self#zero) ~f:(fun acc (_, e) ->
            self#and_ acc @@ self#is_const_expr env e)
      | Aast.Omitted when FileInfo.is_hhi env.Env.in_mode ->
        (* Only allowed in HHI positions where we don't care about the value *)
        (true, self#zero)
      | _ -> (false, Err.naming @@ Naming_error.Illegal_constant pos)

    method private and_ (b1, err1) (b2, err2) = (b1 && b2, self#plus err1 err2)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
