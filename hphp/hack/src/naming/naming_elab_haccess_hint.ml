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
  let in_class t Aast.{ c_name; c_kind; c_final; _ } =
    let elab_haccess_hint = t.Naming_phase_env.elab_haccess_hint in
    let elab_haccess_hint =
      Naming_phase_env.Elab_haccess_hint.
        {
          elab_haccess_hint with
          current_class = Some (c_name, c_kind, c_final);
        }
    in
    Naming_phase_env.{ t with elab_haccess_hint }

  let set_in_context t ~in_context =
    Naming_phase_env.
      {
        t with
        elab_haccess_hint =
          Elab_haccess_hint.{ t.elab_haccess_hint with in_context };
      }

  let set_in_haccess t ~in_haccess =
    Naming_phase_env.
      {
        t with
        elab_haccess_hint =
          Elab_haccess_hint.{ t.elab_haccess_hint with in_haccess };
      }

  let in_haccess
      Naming_phase_env.
        { elab_haccess_hint = Elab_haccess_hint.{ in_haccess; _ }; _ } =
    in_haccess

  let set_in_where_clause t ~in_where_clause =
    Naming_phase_env.
      {
        t with
        elab_haccess_hint =
          Elab_haccess_hint.{ t.elab_haccess_hint with in_where_clause };
      }

  let in_where_clause
      Naming_phase_env.
        { elab_haccess_hint = Elab_haccess_hint.{ in_where_clause; _ }; _ } =
    in_where_clause

  let in_context
      Naming_phase_env.
        { elab_haccess_hint = Elab_haccess_hint.{ in_context; _ }; _ } =
    in_context

  let current_class
      Naming_phase_env.
        { elab_haccess_hint = Elab_haccess_hint.{ current_class; _ }; _ } =
    current_class
end

let on_class_ c ~ctx = (Env.in_class ctx c, Ok c)

let on_where_constraint_hint cstr ~ctx =
  (Env.set_in_where_clause ctx ~in_where_clause:true, Ok cstr)

let on_contexts ctxts ~ctx = (Env.set_in_context ctx ~in_context:true, Ok ctxts)

let on_hint on_error hint ~ctx =
  let res =
    if Env.in_haccess ctx then
      match hint with
      (* TODO[mjt] we appear to be discarding type parameters on `Happly` here
         - should we change the representation of `Haccess` or handle
         erroneous type parameters? *)
      | (pos, Aast.Happly ((tycon_pos, tycon_name), _))
        when String.equal tycon_name SN.Classes.cSelf -> begin
        match Env.current_class ctx with
        | Some (cid, _, _) -> Ok (pos, Aast.Happly (cid, []))
        | _ ->
          Error (hint, Err.naming @@ Naming_error.Self_outside_class tycon_pos)
        (* TODO[mjt] is this ever exercised? The cases is handles appear to
           be a parse errors *)
      end
      | (_, Aast.Happly ((tycon_pos, tycon_name), _))
        when String.(
               equal tycon_name SN.Classes.cStatic
               || equal tycon_name SN.Classes.cParent) ->
        Error
          ( hint,
            Err.naming
            @@ Naming_error.Invalid_type_access_root
                 { pos = tycon_pos; id = Some tycon_name } )
      | (_, Aast.(Hthis | Happly _)) -> Ok hint
      | (_, Aast.Habstr _) when Env.in_where_clause ctx || Env.in_context ctx ->
        Ok hint
      (* TODO[mjt] why are we allow `Hvar`? *)
      | (_, Aast.Hvar _) -> Ok hint
      | (pos, _) ->
        Error
          ( hint,
            Err.naming
            @@ Naming_error.Invalid_type_access_root { pos; id = None } )
    else
      Ok hint
  in
  let ctx =
    match hint with
    | (_, Aast.Haccess _) -> Env.set_in_haccess ctx ~in_haccess:true
    | _ -> Env.set_in_haccess ctx ~in_haccess:false
  in
  match res with
  | Error (hint, err) ->
    on_error err;
    (ctx, Error hint)
  | Ok hint -> (ctx, Ok hint)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_class_ = Some on_class_;
        on_ty_where_constraint_hint = Some on_where_constraint_hint;
        on_ty_contexts = Some on_contexts;
        on_ty_hint = Some (on_hint on_error);
      }
