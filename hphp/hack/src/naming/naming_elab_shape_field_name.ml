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

  val in_class : t -> ('ex, 'en) Aast.class_ -> t

  val current_class : t -> (Ast_defs.id * Ast_defs.classish_kind * bool) option
end = struct
  type t = {
    current_class: (Ast_defs.id * Ast_defs.classish_kind * bool) option;
  }

  let empty = { current_class = None }

  let in_class _ Aast.{ c_name; c_kind; c_final; _ } =
    { current_class = Some (c_name, c_kind, c_final) }

  let current_class { current_class } = current_class
end

(* We permit class constants to be used as shape field names. Here we replace
    uses of `self` with the class to which they refer or `unknown` if the shape
   is not defined within the context of a class *)
let canonical_shape_name current_class sfld =
  match sfld with
  | Ast_defs.SFclass_const ((class_pos, class_name), cst)
    when String.equal class_name SN.Classes.cSelf ->
    (match current_class with
    | Some ((_, class_name), _, _) ->
      Ok (Ast_defs.SFclass_const ((class_pos, class_name), cst))
    | None ->
      let err =
        Err.typing @@ Typing_error.Primary.Self_outside_class class_pos
      in
      Error (Ast_defs.SFclass_const ((class_pos, SN.Classes.cUnknown), cst), err))
  | _ -> Ok sfld

let on_class_ (env, c, err) =
  Naming_phase_pass.Cont.next (Env.in_class env c, c, err)

let on_expr_ (env, expr_, err_acc) =
  let (expr_, err) =
    match expr_ with
    | Aast.Shape fdl ->
      let (fdl, err_opts) =
        List.unzip
        @@ List.map fdl ~f:(fun (nm, v) ->
               let (nm, err_opt) =
                 match canonical_shape_name (Env.current_class env) nm with
                 | Ok nm -> (nm, None)
                 | Error (nm, err) -> (nm, Some err)
               in
               ((nm, v), err_opt))
      in
      let err =
        List.fold_right err_opts ~init:err_acc ~f:(fun err_opt acc ->
            Option.value_map err_opt ~default:acc ~f:(Err.Free_monoid.plus acc))
      in
      (Aast.Shape fdl, err)
    | _ -> (expr_, err_acc)
  in
  Naming_phase_pass.Cont.next (env, expr_, err)

let on_shape_field_info (env, (Aast.{ sfi_name; _ } as sfi), err_acc) =
  match canonical_shape_name (Env.current_class env) sfi_name with
  | Ok sfi_name ->
    Naming_phase_pass.Cont.next (env, Aast.{ sfi with sfi_name }, err_acc)
  | Error (sfi_name, err) ->
    Naming_phase_pass.Cont.finish
      (env, Aast.{ sfi with sfi_name }, Err.Free_monoid.plus err_acc err)

let pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_class_ = Some on_class_;
        on_expr_ = Some on_expr_;
        on_shape_field_info = Some on_shape_field_info;
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
