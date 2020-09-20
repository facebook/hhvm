(***********************************************************************)
(* The OUnit library                                                   *)
(*                                                                     *)
(* Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008              *)
(* Maas-Maarten Zeeman.                                                *)

(*
The package OUnit is copyright by Maas-Maarten Zeeman.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this document and the OUnit software ("the Software"), to
deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

The Software is provided ``as is'', without warranty of any kind,
express or implied, including but not limited to the warranties of
merchantability, fitness for a particular purpose and noninfringement.
In no event shall Maas-Maarten Zeeman be liable for any claim, damages
or other liability, whether in an action of contract, tort or
otherwise, arising from, out of or in connection with the Software or
the use or other dealings in the software.
*)

(***********************************************************************)
(* pad: just harmonized some APIs regarding the 'msg' label *)

let bracket set_up f tear_down () =
  let fixture = set_up () in
  try
    f fixture;
    tear_down fixture
  with e ->
    tear_down fixture;
    raise e

exception Skip of string

let skip_if b msg = if b then raise (Skip msg)

exception Todo of string

let todo msg = raise (Todo msg)

let assert_failure msg = failwith ("OUnit: " ^ msg)

let assert_bool ~msg b = if not b then assert_failure msg

let assert_string str = if not (str = "") then assert_failure str

let assert_equal ?(cmp = ( = )) ?printer ?msg expected actual =
  (* pad: better to use dump by default *)
  let p = Dumper.dump in
  let get_error_string _ =
    match (printer, msg) with
    | (None, None) ->
      Format.sprintf "expected: %s but got: %s" (p expected) (p actual)
    | (None, Some s) ->
      Format.sprintf
        "%s\nnot equal, expected: %s but got: %s"
        s
        (p expected)
        (p actual)
    | (Some p, None) ->
      Format.sprintf "expected: %s but got: %s" (p expected) (p actual)
    | (Some p, Some s) ->
      Format.sprintf "%s\nexpected: %s but got: %s" s (p expected) (p actual)
  in
  if not (cmp expected actual) then assert_failure (get_error_string ())

let raises f =
  try
    f ();
    None
  with e -> Some e

let assert_raises ?msg exn (f : unit -> 'a) =
  let pexn = Printexc.to_string in
  let get_error_string _ =
    let str =
      Format.sprintf
        "expected exception %s, but no exception was raised."
        (pexn exn)
    in
    match msg with
    | None -> assert_failure str
    | Some s -> assert_failure (Format.sprintf "%s\n%s" s str)
  in
  match raises f with
  | None -> assert_failure (get_error_string ())
  | Some e -> assert_equal ?msg ~printer:pexn exn e

(* Compare floats up to a given relative error *)
let cmp_float ?(epsilon = 0.00001) a b =
  abs_float (a -. b) <= epsilon *. abs_float a
  || abs_float (a -. b) <= epsilon *. abs_float b

(* Now some handy shorthands *)
let ( @? ) msg a = assert_bool ~msg a

(* The type of test function *)
type test_fun = unit -> unit

(* The type of tests *)
type test =
  | TestCase of test_fun
  | TestList of test list
  | TestLabel of string * test

(* Some shorthands which allows easy test construction *)
let ( >: ) s t = TestLabel (s, t) (* infix *)

let ( >:: ) s f = TestLabel (s, TestCase f) (* infix *)

let ( >::: ) s l = TestLabel (s, TestList l) (* infix *)

(* Utility function to manipulate test *)
let rec test_decorate g tst =
  match tst with
  | TestCase f -> TestCase (g f)
  | TestList tst_lst -> TestList (List.map (test_decorate g) tst_lst)
  | TestLabel (str, tst) -> TestLabel (str, test_decorate g tst)

(* Return the number of available tests *)
let rec test_case_count test =
  match test with
  | TestCase _ -> 1
  | TestLabel (_, t) -> test_case_count t
  | TestList l -> List.fold_left (fun c t -> c + test_case_count t) 0 l

type node =
  | ListItem of int
  | Label of string

type path = node list

let string_of_node node =
  match node with
  | ListItem n -> string_of_int n
  | Label s -> s

let string_of_path path =
  List.fold_left
    (fun a l ->
      if a = "" then
        l
      else
        l ^ ":" ^ a)
    ""
    (List.map string_of_node path)

(* Some helper function, they are generally applicable *)
(* Applies function f in turn to each element in list. Function f takes
   one element, and integer indicating its location in the list *)
let mapi f l =
  let rec rmapi cnt l =
    match l with
    | [] -> []
    | h :: t -> f h cnt :: rmapi (cnt + 1) t
  in
  rmapi 0 l

let fold_lefti f accu l =
  let rec rfold_lefti cnt accup l =
    match l with
    | [] -> accup
    | h :: t -> rfold_lefti (cnt + 1) (f accup h cnt) t
  in
  rfold_lefti 0 accu l

(* Returns all possible paths in the test. The order is from test case
   to root
*)
let test_case_paths test =
  let rec tcps path test =
    match test with
    | TestCase _ -> [path]
    | TestList tests ->
      List.concat (mapi (fun t i -> tcps (ListItem i :: path) t) tests)
    | TestLabel (l, t) -> tcps (Label l :: path) t
  in
  tcps [] test

(* Test filtering with their path *)
module SetTestPath = Set.Make (String)

let test_filter only test =
  let set_test =
    List.fold_left (fun st str -> SetTestPath.add str st) SetTestPath.empty only
  in
  let foldi f acc lst =
    List.fold_left
      (fun (i, acc) e ->
        let nacc = f i acc e in
        (i + 1, nacc))
      acc
      lst
  in
  let rec filter_test path tst =
    if SetTestPath.mem (string_of_path path) set_test then
      Some tst
    else
      match tst with
      | TestCase _ -> None
      | TestList tst_lst ->
        let (_, ntst_lst) =
          foldi
            (fun i ntst_lst tst ->
              let nntst_lst =
                match filter_test (ListItem i :: path) tst with
                | Some tst -> tst :: ntst_lst
                | None -> ntst_lst
              in
              nntst_lst)
            (0, [])
            tst_lst
        in
        if ntst_lst = [] then
          None
        else
          Some (TestList ntst_lst)
      | TestLabel (lbl, tst) ->
        let ntst = filter_test (Label lbl :: path) tst in
        (match ntst with
        | Some tst -> Some (TestLabel (lbl, tst))
        | None -> None)
  in
  filter_test [] test

(* The possible test results *)
type test_result =
  | RSuccess of path
  | RFailure of path * string
  | RError of path * string
  | RSkip of path * string
  | RTodo of path * string

let is_success = function
  | RSuccess _ -> true
  | RFailure _
  | RError _
  | RSkip _
  | RTodo _ ->
    false

let is_failure = function
  | RFailure _ -> true
  | RSuccess _
  | RError _
  | RSkip _
  | RTodo _ ->
    false

let is_error = function
  | RError _ -> true
  | RSuccess _
  | RFailure _
  | RSkip _
  | RTodo _ ->
    false

let is_skip = function
  | RSkip _ -> true
  | RSuccess _
  | RFailure _
  | RError _
  | RTodo _ ->
    false

let is_todo = function
  | RTodo _ -> true
  | RSuccess _
  | RFailure _
  | RError _
  | RSkip _ ->
    false

let result_flavour = function
  | RError _ -> "Error"
  | RFailure _ -> "Failure"
  | RSuccess _ -> "Success"
  | RSkip _ -> "Skip"
  | RTodo _ -> "Todo"

let result_path = function
  | RSuccess path
  | RError (path, _)
  | RFailure (path, _)
  | RSkip (path, _)
  | RTodo (path, _) ->
    path

let result_msg = function
  | RSuccess _ -> "Success"
  | RError (_, msg)
  | RFailure (_, msg)
  | RSkip (_, msg)
  | RTodo (_, msg) ->
    msg

(* Returns true if the result list contains successes only *)
let rec was_successful results =
  match results with
  | [] -> true
  | RSuccess _ :: t
  | RSkip _ :: t ->
    was_successful t
  | RFailure _ :: _
  | RError _ :: _
  | RTodo _ :: _ ->
    false

(* Events which can happen during testing *)
type test_event =
  | EStart of path
  | EEnd of path
  | EResult of test_result

(* Run all tests, report starts, errors, failures, and return the results *)
let perform_test report test =
  let run_test_case f path =
    try
      f ();
      RSuccess path
    with
    | Failure s -> RFailure (path, s)
    | Skip s -> RSkip (path, s)
    | Todo s -> RTodo (path, s)
    | s ->
      let stack = Printexc.get_backtrace () in
      RError (path, Printexc.to_string s ^ " " ^ stack)
  in
  let rec run_test path results test =
    match test with
    | TestCase f ->
      report (EStart path);
      let result = run_test_case f path in
      report (EResult result);
      report (EEnd path);
      result :: results
    | TestList tests ->
      fold_lefti
        (fun results t cnt -> run_test (ListItem cnt :: path) results t)
        results
        tests
    | TestLabel (label, t) -> run_test (Label label :: path) results t
  in
  run_test [] [] test

(* Function which runs the given function and returns the running time
   of the function, and the original result in a tuple *)
let time_fun f x y =
  let begin_time = Unix.gettimeofday () in
  (Unix.gettimeofday () -. begin_time, f x y)

(* A simple (currently too simple) text based test runner *)
let run_test_tt ?(verbose = false) test =
  let printf = Format.printf in
  let separator1 =
    "======================================================================"
  in
  let separator2 =
    "----------------------------------------------------------------------"
  in
  let string_of_result = function
    | RSuccess _ ->
      if verbose then
        "ok\n"
      else
        "."
    | RFailure (_, _) ->
      if verbose then
        "FAIL\n"
      else
        "F"
    | RError (_, _) ->
      if verbose then
        "ERROR\n"
      else
        "E"
    | RSkip (_, _) ->
      if verbose then
        "SKIP\n"
      else
        "S"
    | RTodo (_, _) ->
      if verbose then
        "TODO\n"
      else
        "T"
  in
  let report_event = function
    | EStart p -> if verbose then printf "%s ... " (string_of_path p)
    | EEnd _ -> ()
    | EResult result -> printf "%s@?" (string_of_result result)
  in
  let print_result_list results =
    List.iter
      (fun result ->
        printf
          "%s\n%s: %s\n\n%s\n%s\n"
          separator1
          (result_flavour result)
          (string_of_path (result_path result))
          (result_msg result)
          separator2)
      results
  in
  (* Now start the test *)
  let (running_time, results) = time_fun perform_test report_event test in
  let errors = List.filter is_error results in
  let failures = List.filter is_failure results in
  let skips = List.filter is_skip results in
  let todos = List.filter is_todo results in
  if not verbose then printf "\n";

  (* Print test report *)
  print_result_list errors;
  print_result_list failures;
  printf "Ran: %d tests in: %.2f seconds.\n" (List.length results) running_time;

  (* Print final verdict *)
  if was_successful results then
    if skips = [] then
      printf "OK"
    else
      printf
        "OK: Cases: %d Skip: %d\n"
        (test_case_count test)
        (List.length skips)
  else
    printf
      "FAILED: Cases: %d Tried: %d Errors: %d Failures: %d Skip:%d Todo:%d\n"
      (test_case_count test)
      (List.length results)
      (List.length errors)
      (List.length failures)
      (List.length skips)
      (List.length todos);

  (* Return the results possibly for further processing *)
  results

(* Call this one from you test suites *)
let run_test_tt_main suite =
  let verbose = ref false in
  let only_test = ref [] in
  Arg.parse
    (Arg.align
       [
         ("-verbose", Arg.Set verbose, " Run the test in verbose mode.");
         ( "-only-test",
           Arg.String (fun str -> only_test := str :: !only_test),
           "path Run only the selected test" );
       ])
    (fun x -> raise (Arg.Bad ("Bad argument : " ^ x)))
    ("usage: " ^ Sys.argv.(0) ^ " [-verbose] [-only-test path]*");

  let nsuite =
    if !only_test = [] then
      suite
    else
      match test_filter !only_test suite with
      | Some tst -> tst
      | None ->
        failwith
          ( "Filtering test "
          ^ String.concat ", " !only_test
          ^ " lead to no test" )
  in
  let result = run_test_tt ~verbose:!verbose nsuite in
  if not (was_successful result) then
    exit 1
  else
    result
