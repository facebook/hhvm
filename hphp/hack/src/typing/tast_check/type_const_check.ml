(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Cls = Decl_provider.Class
open Typing_const_reifiable

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_typeconst_def env { c_tconst_name = (_, name); _ } =
      let cls_opt =
        Decl_entry.bind
          (Decl_entry.of_option_or_doe_not_exist (Tast_env.get_self_id env))
          (Tast_env.get_class env)
      in
      match cls_opt with
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        ()
      | Decl_entry.Found cls -> begin
        match Cls.get_typeconst cls name with
        | None -> ()
        | Some tc ->
          begin
            match tc.ttc_kind with
            | TCAbstract { atc_default = Some ty; _ }
            | TCConcrete { tc_type = ty } ->
              let (tp_pos, enforceable) =
                Option.value_exn (Cls.get_typeconst_enforceability cls name)
              in
              if enforceable then
                Typing_enforceable_hint.validate_type
                  (Tast_env.tast_env_as_typing_env env)
                  (fst tc.ttc_name |> Pos_or_decl.unsafe_to_raw_pos)
                  ty
                  (fun pos ty_info ->
                    Typing_error_utils.add_typing_error
                      ~env:(Tast_env.tast_env_as_typing_env env)
                      Typing_error.(
                        primary
                        @@ Primary.Invalid_enforceable_type
                             {
                               pos;
                               ty_info;
                               kind = `constant;
                               tp_pos;
                               tp_name = name;
                             }))
            | _ -> ()
          end;
          if String.equal tc.ttc_origin (Cls.name cls) then
            Option.iter
              tc.ttc_reifiable
              ~f:(check_reifiable (Tast_env.tast_env_as_typing_env env) tc)
      end
  end
