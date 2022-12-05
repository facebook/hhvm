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

let on_class_ (env, c, err) =
  Naming_phase_pass.Cont.next (Env.in_class env c, c, err)

let on_where_constraint_hint (env, cstr, err) =
  Naming_phase_pass.Cont.next
    (Env.set_in_where_clause env ~in_where_clause:true, cstr, err)

let on_contexts (env, ctxts, err) =
  Naming_phase_pass.Cont.next
    (Env.set_in_context env ~in_context:true, ctxts, err)

let on_hint (env, hint, err_acc) =
  let res =
    if Env.in_haccess env then
      match hint with
      (* TODO[mjt] we appear to be discarding type parameters on `Happly` here
         - should we change the representation of `Haccess` or handle
         erroneous type parameters? *)
      | (pos, Aast.Happly ((tycon_pos, tycon_name), _))
        when String.equal tycon_name SN.Classes.cSelf ->
        begin
          match Env.current_class env with
          | Some (cid, _, _) -> Ok (pos, Aast.Happly (cid, []))
          | _ ->
            Error
              ( (pos, Aast.Herr),
                Err.typing @@ Typing_error.Primary.Self_outside_class tycon_pos
              )
        end
        (* TODO[mjt] is this ever exercised? The cases is handles appear to
           be a parse errors *)
      | (pos, Aast.Happly ((tycon_pos, tycon_name), _))
        when String.(
               equal tycon_name SN.Classes.cStatic
               || equal tycon_name SN.Classes.cParent) ->
        Error
          ( (pos, Aast.Herr),
            Err.naming
            @@ Naming_error.Invalid_type_access_root
                 { pos = tycon_pos; id = Some tycon_name } )
      | (_, Aast.(Hthis | Happly _)) -> Ok hint
      | (_, Aast.Habstr _) when Env.in_where_clause env || Env.in_context env ->
        Ok hint
      (* TODO[mjt] why are we allow `Hvar`? *)
      | (_, Aast.Hvar _) -> Ok hint
      | (pos, _) ->
        Error
          ( (pos, Aast.Herr),
            Err.naming
            @@ Naming_error.Invalid_type_access_root { pos; id = None } )
    else
      Ok hint
  in
  let env =
    match hint with
    | (_, Aast.Haccess _) -> Env.set_in_haccess env ~in_haccess:true
    | _ -> Env.set_in_haccess env ~in_haccess:false
  in
  match res with
  | Error (hint, err) ->
    let err = err :: err_acc in
    Naming_phase_pass.Cont.finish (env, hint, err)
  | Ok hint -> Naming_phase_pass.Cont.next (env, hint, err_acc)

let pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_class_ = Some on_class_;
        on_where_constraint_hint = Some on_where_constraint_hint;
        on_contexts = Some on_contexts;
        on_hint = Some on_hint;
      })
