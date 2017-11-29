(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let type_at_pos tast line char =
  let visitor =
    object (self)
      inherit [(Tast.saved_env * Tast.ty) option] Tast_visitor.visitor as super
      method! on_expr acc e =
        let (pos, ty) = fst e in
        let acc =
          if not (Pos.inside pos line char) then acc else
            match ty with
            | Some ty -> Some (self#saved_env, ty)
            | None -> acc
        in
        super#on_expr acc e
      (* When the expression being applied has a Tfun type, replace that type
       * with its return type. This matches with legacy behavior and is
       * better-suited for IDE hover (at present, since full function types are
       * presented in a way which makes them difficult to read). *)
      method! on_call acc ct e el uel =
        let open Option.Monad_infix in
        let return_type ty =
          let open Typing_defs in
          match snd ty with Tfun ft -> ft.ft_ret | _ -> ty
        in
        let (receiver_pos, _) = fst e in
        if Pos.inside receiver_pos line char
        then super#on_expr acc e >>| fun (env, ty) -> env, return_type ty
        else super#on_call acc ct e el uel
    end
  in
  visitor#on_program None tast

let make_result tcopt (saved_env, ty) =
  let file = Relative_path.create Relative_path.Dummy "<file>" in
  let env = Typing_env.empty tcopt file ~droot:None in
  let env = Tast_expand.restore_saved_env env saved_env in
  Typing_print.full_strip_ns env ty,
  Typing_print.to_json env ty |> Hh_json.json_to_string

let go:
  ServerEnv.env ->
  (ServerUtils.file_input * int * int) ->
  (string * string) option =
fun env pos ->
  let file, line, char = pos in
  let {ServerEnv.tcopt; files_info; _} = env in
  let _, tast = ServerIdeUtils.check_file_input tcopt files_info file in
  type_at_pos tast line char
  |> Option.map ~f:(make_result tcopt)
