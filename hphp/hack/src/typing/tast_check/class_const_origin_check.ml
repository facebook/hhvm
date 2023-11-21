(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env

module StringPair = struct
  type t = string * string [@@deriving ord, eq]
end

module SPSet = Stdlib.Set.Make (StringPair)

(* Check that a constant does not depend on itself when initialized.
 * The cycle can span multiple classes. We're using the decl info
 * that tells us what are the constants *directly* used to initialize
 * `class_name::constant_name` to build a transitive set and check for cycles.
 *
 * Note that some constant are using `self` instead of the current class
 * name, so we always carry the class where refs come from to be able
 * to perform the correct naming resolution.
 *)
let find_cycle env class_name constant_name =
  let get_origin_and_refs class_name constant_name =
    let open Option in
    let cls = Env.get_class env class_name in
    cls >>= fun cls ->
    Env.get_const env cls constant_name >>| fun class_const ->
    (class_const.cc_origin, class_const.cc_refs)
  in
  let target = (class_name, constant_name) in
  (* Todo is a list of pairs: a class name and class constant references
   * for constants within this class. It is used to resolve `self` correctly.
   *)
  let rec spot_target visited todo =
    match todo with
    | [] -> false
    | (current_class, refs) :: todo ->
      (* Normalize refs *)
      let refs =
        List.map
          ~f:(function
            | (Typing_defs.Self, name) -> (current_class, name)
            | (Typing_defs.From class_name, name) ->
              (* Do deal with inherited constants we need to check the
               * origin of constants from the decls.
               *)
              let origin =
                let open Option in
                let cls = Env.get_class env class_name in
                cls >>= fun cls ->
                Env.get_const env cls name >>| fun class_const ->
                class_const.cc_origin
              in
              let origin = Option.value ~default:class_name origin in
              (origin, name))
          refs
      in
      let (spotted, visited, todo) =
        List.fold
          ~init:(false, visited, todo)
          ~f:(fun (spotted, visited, todo) (c_name, name) ->
            let spotted = spotted || StringPair.equal target (c_name, name) in
            if spotted then
              (spotted, visited, todo)
            else if SPSet.mem (c_name, name) visited then
              (spotted, visited, todo)
            else
              let visited = SPSet.add (c_name, name) visited in
              let todo =
                match get_origin_and_refs c_name name with
                | None -> todo
                | Some (origin, refs) -> (origin, refs) :: todo
              in
              (spotted, visited, todo))
          refs
      in
      spotted || spot_target visited todo
  in
  let empty = SPSet.empty in
  let init = get_origin_and_refs class_name constant_name in
  let init = Option.value ~default:(class_name, []) init in
  spot_target empty [init]

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let c_name = snd c.c_name in
      let c_consts = c.c_consts in
      let cls = Env.get_class env c_name in
      match cls with
      | None -> ()
      | Some cls ->
        List.iter c_consts ~f:(fun cc ->
            let cc_name = snd cc.cc_id in
            let cc = Env.get_const env cls cc_name in
            match cc with
            | None -> ()
            | Some cc ->
              (* This class constant may be inherited from an ancestor class, in which
               * case we don't want to fire the error in this file. *)
              Option.iter
                (Tast_env.fill_in_pos_filename_if_in_current_decl env cc.cc_pos)
                ~f:(fun cc_pos ->
                  if find_cycle env c_name cc_name then
                    Typing_error_utils.add_typing_error
                      ~env:(Env.tast_env_as_typing_env env)
                      Typing_error.(
                        primary
                        @@ Primary.Cyclic_class_constant
                             {
                               pos = cc_pos;
                               class_name = c_name;
                               const_name = cc_name;
                             })))
  end
