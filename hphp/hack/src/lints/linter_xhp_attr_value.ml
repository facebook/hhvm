(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module Cls = Decl_provider.Class

(** If [attr_name] is an enum attribute on [cls], return the list of allowed values. *)
let xhp_enum_attr_values env (cls : Cls.t) (attr_name : string) :
    Ast_defs.xhp_enum_value list option =
  let attr_name = ":" ^ attr_name in
  Cls.props cls
  |> List.find ~f:(fun (name, _) -> String.equal attr_name name)
  |> Option.map ~f:(fun (_, { Typing_defs.ce_origin = n; _ }) -> n)
  |> Option.bind ~f:(fun cls_name ->
         Decl_entry.to_option
         @@ Decl_provider.get_class (Tast_env.get_ctx env) cls_name)
  |> Option.bind ~f:(fun cls ->
         SMap.find_opt attr_name (Cls.xhp_enum_values cls))

let split_attr_values (decl_values : Ast_defs.xhp_enum_value list) :
    int list * string list =
  let rec aux dvs ((int_vals, string_vals) as acc) =
    match dvs with
    | Ast_defs.XEV_Int i :: dvs -> aux dvs (i :: int_vals, string_vals)
    | Ast_defs.XEV_String s :: dvs -> aux dvs (int_vals, s :: string_vals)
    | [] -> acc
  in
  aux decl_values ([], [])

(** Format enum values as Hack literals. *)
let attr_value_literals (decl_values : Ast_defs.xhp_enum_value list) :
    string list =
  let as_literal = function
    | Ast_defs.XEV_Int i -> string_of_int i
    | Ast_defs.XEV_String s -> "\"" ^ s ^ "\""
  in
  List.map decl_values ~f:as_literal

(** Best-effort conversion from a Hack integer literal to its integer value. *)
let int_of_hack_literal (literal : string) : int option =
  let parts = String.split ~on:'_' literal in
  let clean_literal = String.concat ~sep:"" parts in
  int_of_string_opt clean_literal

(** If [attr] is initialized with a literal value that isn't in the
    enum declaration, show a lint error. *)
let check_attr_value env (cls : Cls.t) (attr : ('a, 'b) xhp_attribute) : unit =
  match attr with
  | Xhp_simple
      { xs_name = (_, attr_name); xs_expr = (_, attr_val_pos, attr_val); _ } ->
    (match xhp_enum_attr_values env cls attr_name with
    | Some attr_values ->
      let (int_values, string_values) = split_attr_values attr_values in

      (match attr_val with
      | Int i ->
        (match int_of_hack_literal i with
        | Some i ->
          if not (List.mem int_values i ~equal:Int.equal) then
            Lints_errors.invalid_attribute_value
              attr_val_pos
              attr_name
              (attr_value_literals attr_values)
        | None -> ())
      | String s ->
        if not (List.mem string_values s ~equal:String.equal) then
          Lints_errors.invalid_attribute_value
            attr_val_pos
            attr_name
            (attr_value_literals attr_values)
      | _ -> ())
    | None -> ())
  | Xhp_spread _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, _, e_) =
      match e_ with
      | Xml ((_, class_name), attrs, _children) ->
        (match Decl_provider.get_class (Tast_env.get_ctx env) class_name with
        | Decl_entry.Found cls -> List.iter attrs ~f:(check_attr_value env cls)
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          ())
      | _ -> ()
  end
