(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type patches = ServerRenameTypes.patch list

type can_be_captured = bool

module PatchHeap = struct
  include
    SharedMem.Heap
      (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (Relative_path.S)
      (struct
        type t = can_be_captured * Pos.t * Pos.t

        let description =
          "Positions needed to generate dead unsafe cast removal patches"
      end)
end

let patch_location_collection_handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env expr =
      let typing_env = Tast_env.tast_env_as_typing_env env in
      match expr with
      | ( _,
          hole_pos,
          Aast.Hole
            ( (expr_ty, expr_pos, expr),
              _,
              dest_ty,
              (Aast.UnsafeCast _ | Aast.UnsafeNonnullCast) ) )
        when (not @@ Typing_defs.is_any expr_ty)
             && Typing_subtype.is_sub_type
                  typing_env
                  expr_ty
                  (Typing_make_type.locl_like Typing_reason.Rnone dest_ty) ->
        let path = Tast_env.get_file env in
        (* The following shared memory write will only work when the entry for
           `path` is empty. Later updates will be dropped on the floor. This is
           the desired behaviour as patches don't compose and we want to apply
           them one at a time per file. *)
        PatchHeap.add path (Aast_utils.can_be_captured expr, hole_pos, expr_pos)
      | _ -> ()
  end

let generate_patch content (can_be_captured, hole_pos, expr_pos) =
  let text = Pos.get_text_from_pos ~content expr_pos in
  (* Enclose with parantheses to prevent accidental capture of the expression
     if the expression inside the UNSAFE_CAST can be captured. *)
  let text =
    if can_be_captured then
      "(" ^ text ^ ")"
    else
      text
  in
  ServerRenameTypes.Replace
    ServerRenameTypes.{ pos = Pos.to_absolute hole_pos; text }

let get_patches ?(is_test = false) ~files_info ~fold =
  let get_patches_from_file path _ patches =
    match PatchHeap.get path with
    | None -> patches
    | Some patch_info ->
      (* Do not read from the disk in test mode because the file is not written
         down between successive applications of the patch. Instead a
         replacement is placed in memory through the file provider. *)
      let contents =
        File_provider.get_contents ~force_read_disk:(not is_test) path
        |> Option.value_exn
      in
      let new_patch = generate_patch contents patch_info in
      PatchHeap.remove path;
      new_patch :: patches
  in
  fold files_info ~init:[] ~f:get_patches_from_file
