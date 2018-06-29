(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Typing code concerned the <<__PPL>> attribute. *)
open Hh_core
open Typing_defs
open Tast
open Decl_defs

module Env = Tast_env

let has_ppl_attribute c =
  List.exists
    c.c_user_attributes
    (fun { ua_name; _ } -> SN.UserAttributes.uaProbabilisticModel = snd ua_name)

(**
 * Given a class, check the class's direct ancestors to verify that if
 * one member is annotated with the <<__PPL>> attribute, then all of them are.
 *)
let check_ppl_class env c =
  let is_ppl = has_ppl_attribute c in
  let child_class_string = Ast_defs.string_of_class_kind c.c_kind in
  let c_pos = fst c.c_name in
  let error = Errors.extend_ppl c_pos child_class_string is_ppl in
  let decl_env = Env.get_decl_env env in
  let check verb parent_class_string =
    function
    | _, Nast.Happly ((_, name), _) ->
      begin match Decl_env.get_class_dep decl_env name with
        | Some parent_type ->
          if parent_type.dc_ppl != is_ppl
          then error parent_type.dc_pos parent_class_string parent_type.dc_name verb
          else ()
        | None -> ()
      end
    | _ -> () in
  List.iter (c.c_extends) (check "extend" "class");
  List.iter (c.c_implements) (check "implement" "interface");
  List.iter (c.c_uses) (check "use" "trait");
  List.iter (c.c_req_extends) (check "require" "class");
  List.iter (c.c_req_implements) (check "require" "interface")

let handler = object
  inherit Tast_visitor.handler_base

  method! at_class_ env c = check_ppl_class env c
end
