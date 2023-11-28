(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Base
module Env = Tast_env
module Cls = Decl_provider.Class

let check_is_class env ~require_class_check (p, h) =
  match h with
  | Aast.Happly ((_, name), _) -> begin
    match Env.get_class env name with
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      ()
    | Decl_entry.Found cls ->
      let kind = Cls.kind cls in
      let name = Cls.name cls in
      if Ast_defs.is_c_class kind then (
        if Cls.final cls && not require_class_check then
          Errors.add_error
            Nast_check_error.(
              to_user_error @@ Requires_final_class { pos = p; name })
      ) else
        Errors.add_error
          Nast_check_error.(
            to_user_error
            @@ Requires_non_class
                 { pos = p; name; kind = Ast_defs.string_of_classish_kind kind })
  end
  | Aast.Habstr (name, _) ->
    Errors.add_error
      Nast_check_error.(
        to_user_error
        @@ Requires_non_class { pos = p; name; kind = "a generic" })
  | _ ->
    Errors.add_error
      Nast_check_error.(
        to_user_error
        @@ Requires_non_class
             { pos = p; name = "This"; kind = "an invalid type hint" })

let check_is_interface (env, error_verb) (p, h) =
  match h with
  | Aast.Happly ((_, name), _) -> begin
    match Env.get_class env name with
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      ()
    | Decl_entry.Found cls when Ast_defs.is_c_interface (Cls.kind cls) -> ()
    | Decl_entry.Found cls ->
      Errors.add_error
        Nast_check_error.(
          to_user_error
          @@ Non_interface { pos = p; name = Cls.name cls; verb = error_verb })
  end
  | Aast.Habstr _ ->
    Errors.add_error
      Nast_check_error.(
        to_user_error
        @@ Non_interface { pos = p; name = "generic"; verb = error_verb })
  | _ ->
    Errors.add_error
      Nast_check_error.(
        to_user_error
        @@ Non_interface
             { pos = p; name = "invalid type hint"; verb = error_verb })

let check_is_trait env (p, h) =
  match h with
  | Aast.Happly ((_, name), _) ->
    let type_info = Env.get_class env name in
    begin
      match type_info with
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        ()
      | Decl_entry.Found cls when Ast_defs.is_c_trait (Cls.kind cls) -> ()
      | Decl_entry.Found cls ->
        let name = Cls.name cls in
        let kind = Cls.kind cls in
        Errors.add_error
          Nast_check_error.(
            to_user_error
            @@ Uses_non_trait
                 { pos = p; name; kind = Ast_defs.string_of_classish_kind kind })
    end
  | _ -> failwith "assertion failure: trait isn't an Happly"

let hint_happly_to_string h =
  match h with
  | Aast.Happly ((_, name), _) -> Some name
  | _ -> None

let duplicated_used_traits env c =
  let traits = Hashtbl.create (module String) in
  List.iter
    ~f:(fun (p, h) ->
      match hint_happly_to_string h with
      | Some s -> Hashtbl.add_multi traits ~key:s ~data:p
      | None -> ())
    c.c_uses;
  Hashtbl.iteri
    ~f:(fun ~key ~data ->
      if List.length data > 1 then
        let (pos, class_name) = c.c_name in
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Trait_reuse_inside_class
                 {
                   class_name;
                   pos;
                   trait_name = key;
                   occurrences = List.rev_map data ~f:Pos_or_decl.of_raw_pos;
                 }))
    traits

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let (req_extends, req_implements, req_class) = split_reqs c.c_reqs in
      List.iter c.c_uses ~f:(check_is_trait env);
      duplicated_used_traits (Env.tast_env_as_typing_env env) c;
      List.iter req_extends ~f:(check_is_class ~require_class_check:false env);
      List.iter
        c.c_implements
        ~f:(check_is_interface (env, Nast_check_error.Vimplement));
      List.iter
        req_implements
        ~f:(check_is_interface (env, Nast_check_error.Vreq_implement));
      List.iter req_class ~f:(check_is_class ~require_class_check:true env)
  end
