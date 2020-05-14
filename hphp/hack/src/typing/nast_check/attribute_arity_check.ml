(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast

let has_attribute (name : string) (attrs : Nast.user_attribute list) : bool =
  let matches_name { ua_name = (_, attr_name); _ } =
    String.equal attr_name name
  in
  List.exists matches_name attrs

let variadic_pos (f : Nast.fun_) : pos option =
  match f.f_variadic with
  | FVvariadicArg param -> Some param.param_pos
  | FVellipsis p -> Some p
  | FVnonVariadic -> None

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ _ f =
      (* Ban arguments on functions with the __EntryPoint attribute. *)
      if has_attribute "__EntryPoint" f.f_user_attributes then begin
        (match f.f_params with
        | [] -> ()
        | param :: _ -> Errors.entrypoint_arguments param.param_pos);
        match variadic_pos f with
        | Some p -> Errors.entrypoint_arguments p
        | None -> ()
      end
  end
