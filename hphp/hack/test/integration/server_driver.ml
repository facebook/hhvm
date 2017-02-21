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


type response_matcher =
  | StringMatcher of string
  | RegexMatcher of string

let describe_matcher = function
  | StringMatcher s -> "StringMatcher: " ^ s
  | RegexMatcher s -> "RegexMatcher: " ^ s

let matches matcher actual = match matcher with
  | StringMatcher s -> String.equal s actual
  | RegexMatcher s ->
    let re = Str.regexp s in
    try (Str.search_forward re actual 0) >= 0 with
    | Not_found -> false

(** Mostly a convenient GADT to ADT adaptor, because lists cannot be
 * polymorphic. Also groups together RPC commands with its expected
 * responses. *)
type command_response =
  (** A pair of the cmd and the expected result as a string. *)
  | RpcCommand : ('a SCT.t * response_matcher) -> command_response

let cmd_responses_case_0 root = [
  (RpcCommand ((SCT.STATUS), StringMatcher ""));
  (RpcCommand ((SCT.INFER_TYPE
    (ServerUtils.FileName
      ((Path.concat root "foo_1.php") |> Path.to_string), 4, 20 )),
      StringMatcher "int"));
  ]

let cmd_responses_case_1 root = [
  (RpcCommand (
    (SCT.OPEN_FILE (
      ((Path.concat root "foo_1.php") |> Path.to_string),
    (* "foo_1.php", *)
    {|<?hh
        function f() {
            return g() + $missing_var;
        }
    |}
    )), StringMatcher ""));
   (RpcCommand ((SCT.STATUS), RegexMatcher "^Undefined variable.*"));
  ]

exception Invalid_test_case

let get_test_case case_number root = match case_number with
  | 0 -> cmd_responses_case_0 root
  | 1 -> cmd_responses_case_1 root
  | _ -> raise Invalid_test_case

exception Response_doesnt_match_expected of string * response_matcher

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
      let errors = String.concat "\n" errors in
      errors
    | SCT.INFER_TYPE _, response ->
      let to_string = function
        | None -> "None"
        | Some ty -> Printf.sprintf "%s" ty
      in
      let response = to_string response in
      response
    | SCT.EDIT_FILE _, () -> ""
    | SCT.OPEN_FILE _, () -> ""
    | _, _ -> "Some other response"
  end

let send_command_and_read_response conn cmd_response = match cmd_response with
  | RpcCommand (cmd, expected) ->
    let response = ClientIde.rpc conn cmd in
    let response = response_to_string cmd response in
    let is_equal = matches expected response in
    if is_equal then
      ()
    else
      raise (Response_doesnt_match_expected (response, expected))

(** Runs the given case number on the server running on root.
 * Returns the persistent connection so that the server's IDE
 * session state is kept alive. *)
let connect_and_run_case case_number root =
  let test_case = get_test_case case_number root in
  let conn = ClientIde.connect_persistent { ClientIde.root = root }
    ~retries:800 in
  let () = try List.iter (send_command_and_read_response conn) test_case with
  | Response_doesnt_match_expected (actual, expected) ->
    Printf.eprintf "Failed case %d\n%!" case_number;
    Printf.eprintf "Expected: %s. Found: %s" (describe_matcher expected) actual;
    exit 1
  in
  conn
