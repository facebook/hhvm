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
  let in_mode
      Naming_phase_env.{ elab_const_expr = Elab_const_expr.{ in_mode; _ }; _ } =
    in_mode

  let in_enum_class
      Naming_phase_env.
        { elab_const_expr = Elab_const_expr.{ in_enum_class; _ }; _ } =
    in_enum_class

  let enforce_const_expr
      Naming_phase_env.
        { elab_const_expr = Elab_const_expr.{ enforce_const_expr; _ }; _ } =
    enforce_const_expr

  let set_mode t ~in_mode =
    Naming_phase_env.
      {
        t with
        elab_const_expr = Elab_const_expr.{ t.elab_const_expr with in_mode };
      }

  let set_in_enum_class t ~in_enum_class =
    Naming_phase_env.
      {
        t with
        elab_const_expr =
          Elab_const_expr.{ t.elab_const_expr with in_enum_class };
      }

  let set_enforce_const_expr t ~enforce_const_expr =
    Naming_phase_env.
      {
        t with
        elab_const_expr =
          Elab_const_expr.{ t.elab_const_expr with enforce_const_expr };
      }
end

(* We can determine that certain expressions are invalid based on one-level
   pattern matching. We prefer to do this since we can stop the transformation
   early in these cases. For cases where we need to pattern match on the
   expression more deeply, we use the bottom-up pass *)
let on_expr_top_down
    (on_error : Naming_phase_error.t -> unit)
    ((_annot, pos, expr_) as expr)
    ~ctx =
  if not @@ Env.enforce_const_expr ctx then
    (ctx, Ok expr)
  else begin
    match expr_ with
    (* -- Always valid ------------------------------------------------------ *)
    | Aast.(
        ( Id _ | Null | True | False | Int _ | Float _ | String _
        | FunctionPointer _ | Eif _ | Darray _ | Varray _ | Tuple _ | Shape _
        | Upcast _ | Package _ )) ->
      (ctx, Ok expr)
    (* -- Markers ----------------------------------------------------------- *)
    | Aast.(Invalid _ | Hole _) -> (ctx, Ok expr)
    (* -- Handled bottom up ------------------------------------------------- *)
    | Aast.(Nameof _)
    | Aast.(Class_const _) ->
      (ctx, Ok expr)
    (* -- Conditionally valid ----------------------------------------------- *)
    (* NB we can perform this top-down since the all valid hints are already in
       canonical *)
    | Aast.(
        As { expr = _; hint = (_, hint_); is_nullable = _; enforce_deep = _ })
      -> begin
      match hint_ with
      | Aast.(Happly ((pos, _), _)) ->
        on_error (Err.naming @@ Naming_error.Illegal_constant pos);
        (ctx, Error (Err.invalid_expr expr))
      | _ ->
        on_error (Err.naming @@ Naming_error.Illegal_constant pos);
        (ctx, Error (Err.invalid_expr expr))
    end
    | Aast.(Unop (uop, _)) -> begin
      match uop with
      | Ast_defs.(Uplus | Uminus | Utild | Unot) -> (ctx, Ok expr)
      | _ ->
        on_error (Err.naming @@ Naming_error.Illegal_constant pos);
        (ctx, Error (Err.invalid_expr expr))
    end
    | Aast.(Binop { bop; _ }) -> begin
      match bop with
      | Ast_defs.Eq _ ->
        on_error (Err.naming @@ Naming_error.Illegal_constant pos);
        (ctx, Error (Err.invalid_expr expr))
      | _ -> (ctx, Ok expr)
    end
    | Aast.(ValCollection ((_, vc_kind), _, _)) -> begin
      match vc_kind with
      | Aast.(Vec | Keyset) -> (ctx, Ok expr)
      | _ ->
        on_error (Err.naming @@ Naming_error.Illegal_constant pos);
        (ctx, Error (Err.invalid_expr expr))
    end
    | Aast.(KeyValCollection ((_, kvc_kind), _, _)) -> begin
      match kvc_kind with
      | Aast.Dict -> (ctx, Ok expr)
      | _ ->
        on_error (Err.naming @@ Naming_error.Illegal_constant pos);
        (ctx, Error (Err.invalid_expr expr))
    end
    | Aast.(Call { func = (_, _, call_expr_); _ }) -> begin
      match call_expr_ with
      | Aast.(Id (_, nm))
        when String.(
               nm = SN.StdlibFunctions.array_mark_legacy
               || nm = SN.PseudoFunctions.unsafe_cast
               || nm = SN.PseudoFunctions.unsafe_nonnull_cast) ->
        (ctx, Ok expr)
      | _ ->
        on_error (Err.naming @@ Naming_error.Illegal_constant pos);
        (ctx, Error (Err.invalid_expr expr))
    end
    | Aast.Omitted -> begin
      match Env.in_mode ctx with
      | FileInfo.Mhhi -> (ctx, Ok expr)
      | _ ->
        on_error (Err.naming @@ Naming_error.Illegal_constant pos);
        (ctx, Error (Err.invalid_expr expr))
    end
    (* -- Always invalid ---------------------------------------------------- *)
    | Aast.(
        ( This | Lvar _ | Lplaceholder _ | Array_get _ | Await _ | Cast _
        | Class_get _ | Clone _ | Dollardollar _ | ET_Splice _ | Efun _
        | EnumClassLabel _ | ExpressionTree _ | Is _ | Lfun _ | List _
        | Method_caller _ | New _ | Obj_get _ | Pair _ | Pipe _
        | PrefixedString _ | ReadonlyExpr _ | String2 _ | Yield _ | Xml _ )) ->
      on_error (Err.naming @@ Naming_error.Illegal_constant pos);
      (ctx, Error (Err.invalid_expr expr))
    (* -- Unexpected expressions -------------------------------------------- *)
    | Aast.(Import _ | Collection _) -> raise (Err.UnexpectedExpr pos)
  end

(* Handle non-constant expressions which require pattern matching on some
   element of the expression which is not yet transformed in the top-down pass *)
let on_expr_bottom_up on_error ((_annot, pos, expr_) as expr) ~ctx =
  if not @@ Env.enforce_const_expr ctx then
    (ctx, Ok expr)
  else begin
    match expr_ with
    | Aast.(Nameof (_, _, class_id_))
    | Aast.(Class_const ((_, _, class_id_), _)) -> begin
      (* We have to handle this case bottom-up since the class identifier
         will not have been rewritten in the top-down pass *)
      match class_id_ with
      | Aast.CIstatic ->
        on_error (Err.naming @@ Naming_error.Illegal_constant pos);
        (ctx, Error (Err.invalid_expr expr))
      | Aast.(CIparent | CIself | CI _) -> (ctx, Ok expr)
      | Aast.CIexpr (_, _, expr_) -> begin
        match expr_ with
        (* NB this relies on `class_id` elaboration having been applied, if
           it hasn't we would still have `CIstatic` represented as
           `CIexpr (_,_,Id(_,'static'))` *)
        | Aast.(This | Id _) -> (ctx, Ok expr)
        | _ ->
          on_error (Err.naming @@ Naming_error.Illegal_constant pos);
          (ctx, Error (Err.invalid_expr expr))
      end
    end
    | _ -> (ctx, Ok expr)
  end

let on_class_ c ~ctx =
  let in_enum_class =
    match c.Aast.c_kind with
    | Ast_defs.Cenum_class _ -> true
    | Ast_defs.(Cclass _ | Cinterface | Cenum | Ctrait) -> false
  in
  let ctx =
    Env.set_in_enum_class ~in_enum_class
    @@ Env.set_mode ~in_mode:c.Aast.c_mode ctx
  in
  (ctx, Ok c)

let on_gconst cst ~ctx =
  let ctx = Env.set_mode ctx ~in_mode:cst.Aast.cst_mode in
  let ctx = Env.set_enforce_const_expr ctx ~enforce_const_expr:true in
  (ctx, Ok cst)

let on_typedef t ~ctx = (Env.set_mode ctx ~in_mode:t.Aast.t_mode, Ok t)

let on_fun_def fd ~ctx = (Env.set_mode ctx ~in_mode:fd.Aast.fd_mode, Ok fd)

let on_module_def md ~ctx = (Env.set_mode ctx ~in_mode:md.Aast.md_mode, Ok md)

let on_class_const_kind kind ~ctx =
  let enforce_const_expr =
    (not (Env.in_enum_class ctx))
    &&
    match kind with
    | Aast.CCConcrete _ -> true
    | Aast.CCAbstract _ -> false
  in
  let ctx = Env.set_enforce_const_expr ctx ~enforce_const_expr in
  (ctx, Ok kind)

let top_down_pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_class_ = Some on_class_;
        on_ty_gconst = Some on_gconst;
        on_ty_typedef = Some on_typedef;
        on_ty_fun_def = Some on_fun_def;
        on_ty_module_def = Some on_module_def;
        on_ty_class_const_kind = Some on_class_const_kind;
        on_ty_expr = Some (fun expr ~ctx -> on_expr_top_down on_error expr ~ctx);
      }

let bottom_up_pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up
    Aast.Pass.
      {
        id with
        on_ty_expr =
          Some (fun expr ~ctx -> on_expr_bottom_up on_error expr ~ctx);
      }
