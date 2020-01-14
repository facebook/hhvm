(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let make_local_changes () =
  Errors.set_allow_errors_in_default_path true;
  SharedMem.allow_hashtable_writes_by_current_process false;

  Ast_provider.local_changes_push_stack ();
  Decl_provider.local_changes_push_stack ();
  File_provider.local_changes_push_stack ();
  Fixme_provider.local_changes_push_stack ();

  Ide_parser_cache.activate ();

  Naming_table.push_local_changes ();
  ()

let revert_local_changes () =
  Errors.set_allow_errors_in_default_path false;
  SharedMem.allow_hashtable_writes_by_current_process true;

  Ast_provider.local_changes_pop_stack ();
  Decl_provider.local_changes_pop_stack ();
  File_provider.local_changes_pop_stack ();
  Fixme_provider.local_changes_pop_stack ();

  Ide_parser_cache.deactivate ();

  Naming_table.pop_local_changes ();

  SharedMem.invalidate_caches ();
  ()

let path = Relative_path.default
