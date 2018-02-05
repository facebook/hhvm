(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open HoverService

let symbols_at (file, line, char) tcopt =
  let contents = match file with
    | ServerUtils.FileName file_name ->
      let relative_path = Relative_path.(create Root file_name) in
      File_heap.get_contents relative_path
    | ServerUtils.FileContent content -> Some content
  in
  match contents with
  | None -> []
  | Some contents -> ServerIdentifyFunction.go contents line char tcopt

let type_at (file, line, char) tcopt files_info =
  let open Typing_defs in
  let _, tast = ServerIdeUtils.check_file_input tcopt files_info file in
  match ServerInferType.type_at_pos tast line char with
  | Some (_, (_, Tanon _) as infer_type_results1) ->
    (* The Tanon type doesn't include argument or return types, so it's
       displayed as "[fun]". To try to show something a little more useful, we
       call `returned_type_at_pos`. This will give us the function's return type
       (if it is being invoked). *)
    Some (Option.value
      (ServerInferType.returned_type_at_pos tast line char)
      ~default:infer_type_results1)
  | results -> results

let make_hover_info env_and_ty (occurrence, _definition) =
  let open SymbolOccurrence in
  let open Typing_defs in
  begin match occurrence, env_and_ty with
    | { name; _ }, None
    | { type_ = Class; name; _ }, _ -> Some (Utils.strip_ns name)

    | { type_ = Function; name; _ }, Some (env, (_, Tfun ft))
    | { type_ = Method (_, name); _ }, Some (env, (_, Tfun ft))
    | { type_ = Property (_, name); _ }, Some (env, (_, Tfun ft))
    | { type_ = ClassConst (_, name); _ }, Some (env, (_, Tfun ft))
    | { type_ = GConst; name; _ }, Some (env, (_, Tfun ft)) ->
      Some (Typing_print.full_func_strip_ns env ft name)

    | _, Some (env, ty) -> Some (Typing_print.full_strip_ns env ty)
  end
  |> Option.map ~f:(fun info -> { info = info; doc_block = None })

let go env (file, line, char) =
  let position = (file, line, char) in
  let ServerEnv.{ tcopt; files_info; _ } = env in
  let identities = symbols_at position tcopt in
  let env_and_ty = type_at position tcopt files_info in
  (* There are legitimate cases where we expect to have no identities returned,
     so just format the type. *)
  match identities with
  | [] ->
    begin match env_and_ty with
    | Some (env, ty) -> [{ info = Typing_print.full_strip_ns env ty; doc_block = None }]
    | None -> []
    end
  | identities ->
    identities
    |> List.filter_map ~f:(make_hover_info env_and_ty)
