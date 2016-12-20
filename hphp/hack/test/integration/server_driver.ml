(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * Does a bunch of things to a directory and/or a Hack server running on that
 * directory such as deleting/modifying files on disk, connecting to the server
 * and making requests, reading results from those requests, etc. We need this
 * to automate things for integration testing when we want to avoid python
 * (since IDE requests would be strings in python making integration tests
 * brittle and hard to update when the API changes), and when we can't use
 * integration_ml test because we need to interact with a server that is
 * "more real" than the one offered there.
 *)

module SCT = ServerCommandTypes

module Args = struct

  type t = {
    root : Path.t;
    test_case : int;
  }

  let usage = Printf.sprintf "Usage: %s [WWW DIRECTORY]\n" Sys.argv.(0)

  let parse () =
    let root = ref None in
    let test_case = ref 0 in
    let options = [
      "--test-case", Arg.Int (fun x -> test_case := x),
      "The test case to run. See also cmd_responses_case_[n]";
    ] in
    let () = Arg.parse options (fun s -> root := (Some s)) usage in
    let root = ClientArgsUtils.get_root !root in
    {
      root = root;
      test_case = !test_case;
    }

  let root args = args.root

end

(** Mostly a convenient GADT to ADT adaptor, because lists cannot be
 * polymorphic. Also groups together RPC commands with its expected
 * responses. *)
type command_response =
  (** A pair of the cmd and the expected result as a string. *)
  | RpcCommand : ('a SCT.t * string) -> command_response

let cmd_responses_case_0 args = [
  (RpcCommand ((SCT.STATUS), ""));
  (RpcCommand ((SCT.INFER_TYPE
    (ServerUtils.FileName
      ((Path.concat (Args.root args) "foo_1.php") |> Path.to_string), 4, 20 )),
      "line 3, character 23 - line 3, character 25, int"));
  ]

exception Invalid_test_case

let get_test_case args = match args.Args.test_case with
  | 0 -> cmd_responses_case_0 args
  | _ -> raise Invalid_test_case

exception Response_doesnt_match_expected of string * string

(** We make a string representation of the response, then compare that to
 * the expected. It also allows us to print a useful error message
 * if they are not equal.
 *
 * Note: we must pass in the actual commannd used so we can use the tag
 * in the GADT so we know the type of the response. *)
let response_to_string : type a. a SCT.t -> a -> string = begin
  fun cmd response -> match cmd, response with
    | SCT.STATUS, (_, errors) ->
      let errors = List.map (fun e -> Errors.to_string e) errors in
      let errors = List.fold_left (fun acc e -> acc ^ "\n" ^ e) "" errors in
      errors
    | SCT.INFER_TYPE _, response ->
      let to_string = function
        | None, None -> "None, None"
        | None, Some ty -> "None, " ^ ty
        | Some p, None -> Pos.multiline_string_no_file p ^ " , None"
        | Some p, Some ty ->
          Printf.sprintf "%s, %s" (Pos.multiline_string_no_file p) ty
      in
      let response = to_string response in
      response
    | _, _ -> "Some other response"
  end

let send_command_and_read_response conn cmd_response = match cmd_response with
  | RpcCommand (cmd, expected) ->
    let response = ClientIde.rpc conn cmd in
    let response = response_to_string cmd response in
    let is_equal = response = expected in
    if is_equal then
      ()
    else
      raise (Response_doesnt_match_expected (response, expected))

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  let args = Args.parse () in
  HackEventLogger.client_init @@ Args.root args;
  let conn = ClientIde.connect_persistent { ClientIde.root = Args.root args }
    ~retries:800 in
  let test_case = get_test_case args in
  try List.iter (send_command_and_read_response conn) test_case with
  | Response_doesnt_match_expected (actual, expected) ->
    Printf.eprintf "Failed case %d\n%!" args.Args.test_case;
    Printf.eprintf "Expected: %s. Found: %s" expected actual;
    exit 1
