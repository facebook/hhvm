(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Integration_test_base_types
module Test = Integration_test_base

let test () =
  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in
  let (env, _) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          persistent_client_request =
            Some
              (UncleanDisconect
                 (ServerCommandTypes.STATUS
                    { ignore_ide = false; remote = false; max_errors = None }));
        })
  in
  match env.ServerEnv.persistent_client with
  | None -> ()
  | Some _ -> failwith "expected persistent client to be disconnected"
