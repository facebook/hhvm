(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* Implement the checks from the Safe Abstract proposal,
   * which makes the combination of "abstract" and "static" safer
   * (https://fburl.com/hack-safe-abstract) *)
open Hh_prelude

let handler =
  let current_method = ref None in

  object
    inherit Tast_visitor.handler_base

    method! at_fun_def _env _fun_def = current_method := None

    method! at_method_ _env m = current_method := Some m

    method! at_expr env expr =
      match
        Safe_abstract_internal.calc_warnings
          env
          expr
          ~current_method:!current_method
      with
      | Some (_pos_kind, pos, warnings) ->
        List.iter warnings ~f:(fun warning ->
            Typing_warning_utils.add
              (Tast_env.tast_env_as_typing_env env)
              (pos, Typing_warning.Safe_abstract, warning))
      | None -> ()
  end
