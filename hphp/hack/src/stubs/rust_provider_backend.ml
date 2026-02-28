(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

let set _ : unit = failwith "unimplemented"

let push_local_changes _ : unit = failwith "unimplemented"

let pop_local_changes _ : unit = failwith "unimplemented"

type find_symbol_fn = string -> (FileInfo.pos * FileInfo.name_type) option

type ctx_proxy = {
  get_entry_contents: Relative_path.t -> string option;
  is_pos_in_ctx: FileInfo.pos -> bool;
  find_fun_canon_name_in_context: string -> string option;
  find_type_canon_name_in_context: string -> string option;
  find_const_in_context: find_symbol_fn;
  find_fun_in_context: find_symbol_fn;
  find_type_in_context: find_symbol_fn;
  find_module_in_context: find_symbol_fn;
}

module Decl = struct
  let direct_decl_parse_and_cache _ _ _ = failwith "unimplemented"

  let add_shallow_decls _ _ = failwith "unimplemented"

  let get_fun _ _ _ = failwith "unimplemented"

  let get_shallow_class _ _ _ = failwith "unimplemented"

  let get_typedef _ _ _ = failwith "unimplemented"

  let get_gconst _ _ _ = failwith "unimplemented"

  let get_module _ _ _ = failwith "unimplemented"

  let get_folded_class _ _ _ = failwith "unimplemented"

  let declare_folded_class _ _ _ = failwith "unimplemented"

  let get_old_shallow_classes_batch _ _ = failwith "unimplemented"

  let get_old_defs _ _ = failwith "unimplemented"

  let oldify_defs _ _ : unit = failwith "unimplemented"

  let remove_defs _ _ : unit = failwith "unimplemented"

  let remove_old_defs _ _ : unit = failwith "unimplemented"
end

module File = struct
  type file_type =
    | Disk of string
    | Ide of string

  let get _ _ = failwith "unimplemented"

  let get_contents _ _ = failwith "unimplemented"

  let provide_file_for_tests _ _ _ = failwith "unimplemented"

  let provide_file_for_ide _ _ _ = failwith "unimplemented"

  let provide_file_hint _ _ _ = failwith "unimplemented"

  let remove_batch _ _ = failwith "unimplemented"
end

module Naming = struct
  module Types = struct
    let add _ _ _ = failwith "unimplemented"

    let get_pos _ _ _ = failwith "unimplemented"

    let remove_batch _ _ = failwith "unimplemented"

    let get_canon_name _ _ _ = failwith "unimplemented"
  end

  module Funs = struct
    let add _ _ _ = failwith "unimplemented"

    let get_pos _ _ _ = failwith "unimplemented"

    let remove_batch _ _ = failwith "unimplemented"

    let get_canon_name _ _ _ = failwith "unimplemented"
  end

  module Consts = struct
    let add _ _ _ = failwith "unimplemented"

    let get_pos _ _ _ = failwith "unimplemented"

    let remove_batch _ _ = failwith "unimplemented"
  end

  module Modules = struct
    let add _ _ _ = failwith "unimplemented"

    let get_pos _ _ _ = failwith "unimplemented"

    let remove_batch _ _ = failwith "unimplemented"
  end

  let get_db_path _ = failwith "unimplemented"

  let set_db_path _ _ = failwith "unimplemented"

  let get_filenames_by_hash _ _ = failwith "unimplemented"
end
