(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Simple test runner to verify Member_not_found pattern matching works *)

module Eval = Custom_error_eval

let test_instance_method_pattern () =
  let class_name = "MyClass" in
  let method_name = "nonexistentMethod" in

  let prim_err =
    Typing_error.Primary.Member_not_found
      {
        pos = Pos.none;
        kind = `method_;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = method_name;
        hint = lazy None;
        reason = lazy [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Instance_only;
               patt_kind = Method_only;
               patt_class_name = Patt_string.Exactly class_name;
               patt_member_name = Patt_string.Exactly method_name;
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Instance method not found in class"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let result = Eval.eval_typing_error custom_config ~err in
  Printf.printf
    "Test result: %s\n"
    (match result with
    | [[Core.Either.First msg]] -> "SUCCESS: " ^ msg
    | [] -> "FAILED: No match"
    | _ -> "FAILED: Unexpected result")

let () = test_instance_method_pattern ()
