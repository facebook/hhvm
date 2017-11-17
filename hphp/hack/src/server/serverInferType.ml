(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let go:
  ServerEnv.env ->
  (ServerUtils.file_input * int * int) ->
  (string * string) option =
fun env (file, line, char) ->
  let {ServerEnv.tcopt; files_info; _} = env in
  let _, tast = ServerIdeUtils.check_file_input tcopt files_info file in
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
    end
  in
  visitor#on_program None tast |> Option.map ~f:begin fun (saved_env, ty) ->
    let file = Relative_path.create Relative_path.Dummy "<file>" in
    let tenv = Typing_env.empty tcopt file ~droot:None in
    let tenv = Tast_expand.restore_saved_env tenv saved_env in
    Typing_print.full_strip_ns tenv ty,
    Typing_print.to_json tenv ty |> Hh_json.json_to_string
  end
