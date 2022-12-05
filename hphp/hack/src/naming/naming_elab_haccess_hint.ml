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

module Env : sig
  type t

  val empty : t

  val in_where_clause : t -> bool

  val set_in_where_clause : t -> in_where_clause:bool -> t

  val in_context : t -> bool

  val set_in_context : t -> in_context:bool -> t

  val in_class : t -> (_, _) Aast.class_ -> t

  val current_class : t -> (Ast_defs.id * Ast_defs.classish_kind * bool) option
end = struct
  type t = {
    current_class: (Ast_defs.id * Ast_defs.classish_kind * bool) option;
    in_where_clause: bool;
    in_context: bool;
  }

  let empty =
    { current_class = None; in_where_clause = false; in_context = false }

  let in_class t Aast.{ c_name; c_kind; c_final; _ } =
    { t with current_class = Some (c_name, c_kind, c_final) }

  let set_in_context t ~in_context = { t with in_context }

  let set_in_where_clause t ~in_where_clause = { t with in_where_clause }

  let in_where_clause { in_where_clause; _ } = in_where_clause

  let in_context { in_context; _ } = in_context

  let current_class { current_class; _ } = current_class
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
  match hint with
  | (outer_pos, Aast.Haccess (inner_hint, ids)) ->
    let res =
      match inner_hint with
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
      | (_, Aast.(Hthis | Happly _)) -> Ok inner_hint
      | (_, Aast.Habstr _) when Env.in_where_clause env || Env.in_context env ->
        Ok inner_hint
      (* TODO[mjt] why are we allow `Hvar`? *)
      | (_, Aast.Hvar _) -> Ok inner_hint
      | (pos, _) ->
        Error
          ( (pos, Aast.Herr),
            Err.naming
            @@ Naming_error.Invalid_type_access_root { pos; id = None } )
    in
    begin
      match res with
      | Error (inner_hint, err) ->
        let hint = (outer_pos, Aast.Haccess (inner_hint, ids)) in
        let err = Err.Free_monoid.plus err_acc err in
        Naming_phase_pass.Cont.finish (env, hint, err)
      | Ok inner_hint ->
        let hint = (outer_pos, Aast.Haccess (inner_hint, ids)) in
        Naming_phase_pass.Cont.next (env, hint, err_acc)
    end
  | _ -> Naming_phase_pass.Cont.next (env, hint, err_acc)

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

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
