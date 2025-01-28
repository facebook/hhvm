(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type insert_patch = {
  pos: Pos.absolute;
  text: string;
}

type patch =
  | Insert of insert_patch
  | Remove of Pos.absolute
  | Replace of insert_patch

type action =
  | ClassRename of string * string (* old_name * new_name *)
  | ClassConstRename of string * string * string
  (* class_name * old_name * new_name *)
  | MethodRename of {
      class_name: string;
      old_name: string;
      new_name: string;
    }
  | FunctionRename of {
      old_name: string;
      new_name: string;
    }
  | LocalVarRename of {
      filename: Relative_path.t;
      file_content: string;
      pos: File_content.Position.t;
      new_name: string;
    }

type deprecated_wrapper_function_ref =
  | DeprecatedStaticMethodRef
  | DeprecatedNonStaticMethodRef
  | DeprecatedFunctionRef

val get_pos : patch -> Pos.absolute

(** Compare the positions of two patches. Any texts are ignored. *)
val compare_result : patch -> patch -> int

(** [write_string_to_file filename content] *)
val write_string_to_file : string -> string -> unit

(** [write_patches_to_buffer buffer content patches]
  applies the [patches] to [content] and dump the result in [buffer] *)
val write_patches_to_buffer : Buffer.t -> string -> patch list -> unit

(** [map_patches_to_filename patches patch] adds [patch] to the
  filename-indexed [patches] based off of the patch's position filename. *)
val map_patches_to_filename : patch list SMap.t -> patch -> patch list SMap.t

val apply_patches_to_file_contents :
  string Relative_path.Map.t -> patch list -> string Relative_path.Map.t
