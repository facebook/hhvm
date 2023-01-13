(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let make_env
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) (fn : Relative_path.t) :
    unit =
  match Direct_decl_utils.direct_decl_parse_and_cache ctx fn with
  | None -> ()
  | Some parsed_file ->
    List.iter parsed_file.Direct_decl_utils.pfh_decls ~f:(function
        | (name, Shallow_decl_defs.Class _, _) ->
          (match Provider_context.get_backend ctx with
          | Provider_backend.Rust_provider_backend backend ->
            Rust_provider_backend.Decl.declare_folded_class backend name
          | _ ->
            let (_ : _ option) =
              Decl_folded_class.class_decl_if_missing ~sh ctx name
            in
            ())
        | _ -> ())
