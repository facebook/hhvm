(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Aast

(* Helper function to compute digest of a string *)
let compute_digest str = Md5.digest_string str |> Md5.to_hex

let report_error env err =
  Tast_env.add_typing_error ~env @@ Typing_error.simplihack err

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_user_attribute env { ua_name; ua_params } =
      if
        String.equal
          (snd ua_name)
          Naming_special_names.UserAttributes.uaSimpliHack
      then
        match ua_params with
        | [prompt] ->
          (* We evaluate the prompt in order to record dependencies, though we do not include
             the hash of the prompt in the error message. *)
          let prompt_value = Simplihack_interpreter.eval env prompt in
          let err =
            match prompt_value with
            | Ok _ ->
              (* Report a SimpliHackRunPrompt error. This error should point to the position of the prompt expression
                 and tell the user to run an AI Code Action by clicking on that position in the file.
              *)
              let pos = Tast.get_position prompt in
              Typing_error.Primary.SimpliHack.Run_prompt { pos }
            | Error err -> err
          in
          report_error env err
        | [prompt; hash] ->
          (*
          Evalute prompt and hash using Simplihack_interpreter.eval. If both evaluate to a string then do the following.
          Hash is a Digest encoded as a string.
          Prompt is a string, convert it to a digest.

          Compare the two Digest, if they are different report a SimpliHackRerunPrompt error. The error message should contain
          the two digests and suggest to re-run the AI Code Action
          *)
          let prompt_pos = Tast.get_position prompt in

          (* Evaluate prompt and hash expressions *)
          let prompt_value = Simplihack_interpreter.eval env prompt in
          let hash_value = Simplihack_interpreter.eval env hash in
          (match (prompt_value, hash_value) with
          | (Result.Ok prompt_str, Result.Ok expected_digest) ->
            (* Compute digest of prompt string *)
            let prompt_digest = compute_digest prompt_str in

            (* Compare digests *)
            if not (String.equal prompt_digest expected_digest) then
              report_error env
              @@ Typing_error.Primary.SimpliHack.Rerun_prompt
                   { pos = prompt_pos; prompt_digest; expected_digest }
          | (err1, err2) ->
            Result.iter_error ~f:(report_error env) err1;
            Result.iter_error ~f:(report_error env) err2)
        | _ -> ()
  end
