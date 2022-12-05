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
    current_class: (Ast_defs.id * Ast_defs.classish_kind * bool) option;
    in_where_clause: bool;
    in_context: bool;
  }

  let empty =
    { current_class = None; in_where_clause = false; in_context = false }

  let in_class t Aast.{ c_name; c_kind; c_final; _ } =
    { t with current_class = Some (c_name, c_kind, c_final) }
end

let visitor =
  object (self)
    inherit [_] Naming_visitors.mapreduce as super

    method! on_class_ env c = super#on_class_ (Env.in_class env c) c

    method! on_where_constraint_hint env cstr =
      let env = Env.{ env with in_where_clause = true } in
      super#on_where_constraint_hint env cstr

    method! on_contexts env ctxts =
      let env = Env.{ env with in_context = true } in
      super#on_contexts env ctxts

    method! on_Haccess env hint ids =
      let (hint, err) =
        match hint with
        (* TODO[mjt] we appear to be discarding type parameters on `Happly` here
           - should we change the representation of `Haccess` or handle
           erroneous type parameters? *)
        | (pos, Aast.Happly ((tycon_pos, tycon_name), _))
          when String.equal tycon_name SN.Classes.cSelf ->
          begin
            match env.Env.current_class with
            | Some (cid, _, _) -> ((pos, Aast.Happly (cid, [])), self#zero)
            | _ ->
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
          ( (pos, Aast.Herr),
            Err.naming
            @@ Naming_error.Invalid_type_access_root
                 { pos = tycon_pos; id = Some tycon_name } )
        | (_, Aast.(Hthis | Happly _)) -> (hint, self#zero)
        | (_, Aast.Habstr _) when env.Env.in_where_clause || env.Env.in_context
          ->
          (hint, self#zero)
        (* TODO[mjt] why are we allow `Hvar`? *)
        | (_, Aast.Hvar _) -> (hint, self#zero)
        | (pos, _) ->
          ( (pos, Aast.Herr),
            Err.naming
            @@ Naming_error.Invalid_type_access_root { pos; id = None } )
      in
      let (hint, super_err) = super#on_Haccess env hint ids in
      (hint, self#plus err super_err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
