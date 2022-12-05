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
  }

  let empty = { current_class = None }

  let in_class Aast.{ c_name; c_kind; c_final; _ } =
    { current_class = Some (c_name, c_kind, c_final) }
end

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    method! on_class_ _env c = super#on_class_ (Env.in_class c) c

    method! on_Shape env fdl =
      let (fdl, err) =
        super#on_list (self#on_fst self#canonical_shape_name) env fdl
      in
      let (fdl, super_err) = super#on_Shape env fdl in
      (fdl, self#plus err super_err)

    method! on_shape_field_info env (Aast.{ sfi_name; _ } as sfi) =
      let (sfi_name, err) = self#canonical_shape_name env sfi_name in
      let (sfi, super_err) =
        super#on_shape_field_info env Aast.{ sfi with sfi_name }
      in
      (sfi, self#plus err super_err)

    (* We permit class constants to be used as shape field names. Here we replace
        uses of `self` with the class to which they refer or `unknown` if the shape
       is not defined within the context of a class *)
    method private canonical_shape_name Env.{ current_class } =
      function
      (* TODO[mjt] int field names appear to be a parse error? *)
      | Ast_defs.SFlit_int (pos, s) -> (Ast_defs.SFlit_int (pos, s), self#zero)
      | Ast_defs.SFlit_str (pos, s) -> (Ast_defs.SFlit_str (pos, s), self#zero)
      | Ast_defs.SFclass_const ((class_pos, class_name), (const_pos, const_name))
        ->
        (* e.g. Foo::BAR or self::BAR. The first tuple is the use of Foo, second is the use of BAR *)
        (* We will resolve class-name 'self' *)
        let (class_name, err) =
          if String.equal class_name SN.Classes.cSelf then
            match current_class with
            | Some ((_class_decl_pos, class_name), _, _) ->
              (class_name, self#zero)
            | None ->
              let err =
                Err.typing @@ Typing_error.Primary.Self_outside_class class_pos
              in
              (SN.Classes.cUnknown, err)
          else
            (class_name, self#zero)
        in
        ( Ast_defs.SFclass_const
            ((class_pos, class_name), (const_pos, const_name)),
          err )

    method private on_fst f env (fst, snd) =
      let (fst, err) = f env fst in
      ((fst, snd), err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
