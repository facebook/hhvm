(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Env = struct
  let in_class_or_fun_def
      Naming_phase_env.
        {
          validate_toplevel_statement =
            Validate_toplevel_statement.{ in_class_or_fun_def; _ };
          _;
        } : bool =
    in_class_or_fun_def

  let set_in_class_or_fun_def naming_phase_env =
    Naming_phase_env.
      {
        naming_phase_env with
        validate_toplevel_statement =
          Validate_toplevel_statement.{ in_class_or_fun_def = true };
      }
end

let on_stmt on_error stmt ~ctx =
  (match stmt with
  | (_, Aast.Markup _) -> ()
  | (pos, _) when not (Env.in_class_or_fun_def ctx) ->
    on_error (Naming_phase_error.naming (Naming_error.Toplevel_statement pos))
  | _ -> ());
  (ctx, Ok stmt)

let on_class_or_fun_def elem ~ctx = (Env.set_in_class_or_fun_def ctx, Ok elem)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_class_ = Some on_class_or_fun_def;
        on_ty_fun_def = Some on_class_or_fun_def;
        (* Note: we used neither on_ty_def nor on_program because they are unreachable
           * from Typing_check_job.calc_errors_and_tast
        *)
        on_ty_stmt = Some (fun stmt ~ctx -> on_stmt on_error stmt ~ctx);
      }
