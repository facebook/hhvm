(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

 type patch =
  | Insert of insert_patch
  | Remove of Pos.absolute
  | Replace of insert_patch

and insert_patch = {
  pos: Pos.absolute;
  text: string;
}

type action =
  | ClassRename of string * string (* old_name * new_name *)
  | ClassConstRename of string * string * string
    (* class_name * old_name * new_name *)
  | MethodRename of {
      filename: string option;
      definition: string SymbolDefinition.t option;
      class_name: string;
      old_name: string;
      new_name: string;
    }
  | FunctionRename of string * string (* old_name * new_name *)
  | LocalVarRename of {
      filename: Relative_path.t;
      file_content: string;
      line: int;
      char: int;
      new_name: string;
    }
