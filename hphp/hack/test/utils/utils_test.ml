open Hh_prelude
open Asserter

let assert_ns_split name assert_left assert_right =
  let (left, right) = Utils.split_ns_from_name name in
  String_asserter.assert_equals left assert_left "Namespace is wrong";
  String_asserter.assert_equals right assert_right "Namespace is wrong"

let test_namespace_splitter () =
  assert_ns_split "HH\\Lib\\Str\\Format" "HH\\Lib\\Str\\" "Format";
  assert_ns_split "NameWithoutANamespace" "\\" "NameWithoutANamespace";
  assert_ns_split "HH\\Lib\\Str\\" "HH\\Lib\\Str\\" "";
  assert_ns_split
    "\\HH\\Lib\\Hellothisisafunction"
    "\\HH\\Lib\\"
    "Hellothisisafunction";
  true

let assert_expand_ns str map expected : unit =
  let r = Utils.expand_namespace map str in
  String_asserter.assert_equals
    r
    expected
    "Expanded namespace does not match expected"

let test_expand_namespace () =
  let nsmap =
    [
      ("Dict", "HH\\Lib\\Dict");
      ("Vec", "HH\\Lib\\Vec");
      ("Keyset", "HH\\Lib\\Keyset");
      ("C", "HH\\Lib\\C");
      ("Str", "HH\\Lib\\Str");
    ]
  in
  assert_expand_ns "Str\\join" nsmap "\\HH\\Lib\\Str\\join";
  assert_expand_ns "HH\\Lib\\Str\\join" nsmap "\\HH\\Lib\\Str\\join";
  assert_expand_ns "\\HH\\Lib\\Str\\join" nsmap "\\HH\\Lib\\Str\\join";
  assert_expand_ns "global_func" nsmap "\\global_func";
  true

let test_strip_namespace () =
  String_asserter.assert_equals
    (Utils.strip_both_ns "\\MyClass")
    "MyClass"
    "Strip both should remove Hack namespaces";
  String_asserter.assert_equals
    (Utils.strip_both_ns ":xhp:foo")
    "xhp:foo"
    "Strip both should remove XHP namespaces";
  String_asserter.assert_equals
    (Utils.strip_both_ns "MyClass")
    "MyClass"
    "Strip both should leave unchanged normal strings";
  String_asserter.assert_equals
    (Utils.strip_both_ns "\\:MyClass")
    "MyClass"
    "Strip both should remove both \\ and :";
  String_asserter.assert_equals
    (Utils.strip_xhp_ns "\\MyClass")
    "\\MyClass"
    "Strip xhp should leave unchanged Hack namespaces";
  String_asserter.assert_equals
    (Utils.strip_xhp_ns ":xhp:foo")
    "xhp:foo"
    "Strip xhp should remove XHP namespaces";
  String_asserter.assert_equals
    (Utils.strip_xhp_ns "MyClass")
    "MyClass"
    "Strip xhp should leave unchanged normal strings";
  true

let assert_throws : 'a 'b. ('b -> 'a) -> 'b -> string -> string -> unit =
 fun f arg exp message ->
  let e =
    try
      let _ = f arg in
      "[no exception]"
    with
    | e -> Caml.Printexc.to_string e
  in
  if not (String.is_substring ~substring:exp e) then begin
    Printf.eprintf "%s.\nExpected it to throw '%s' but got '%s'\n" message exp e;
    assert false
  end;
  ()

let test_telemetry_test () =
  let sub_telemetry =
    Telemetry.create ()
    |> Telemetry.int_ ~key:"a" ~value:15
    |> Telemetry.int_opt ~key:"b" ~value:None
    |> Telemetry.string_ ~key:"c" ~value:"oops"
  in
  let telemetry =
    Telemetry.create ()
    |> Telemetry.int_ ~key:"d" ~value:12
    |> Telemetry.object_ ~key:"e" ~value:sub_telemetry
  in

  Int_asserter.assert_equals
    (Telemetry_test_utils.int_exn telemetry "d")
    12
    "int_exn d should be 12";
  assert_throws
    (Telemetry_test_utils.int_exn telemetry)
    "e"
    "Assertion failed"
    "int_exn e should throw";
  assert_throws
    (Telemetry_test_utils.int_exn telemetry)
    ""
    "empty path"
    "int_exn '' should throw";
  Int_asserter.assert_equals
    (Telemetry_test_utils.int_exn telemetry "e.a")
    15
    "int_exn e.a should be 15";
  assert_throws
    (Telemetry_test_utils.int_exn telemetry)
    "e.b"
    "Assertion failed"
    "int_exn e.b should throw";
  assert_throws
    (Telemetry_test_utils.int_exn telemetry)
    "e.d"
    "not found"
    "int_exn e.d should throw";
  true

let test_telemetry_diff () =
  (* different values *)
  let current1 = Telemetry.create () |> Telemetry.int_ ~key:"a" ~value:1 in
  let prev1 = Telemetry.create () |> Telemetry.int_ ~key:"a" ~value:2 in
  let diff1 = Telemetry.diff ~all:true current1 ~prev:prev1 in
  Int_asserter.assert_equals
    (Telemetry_test_utils.int_exn diff1 "a")
    1
    "diff1 a should be -1";
  Int_asserter.assert_equals
    (Telemetry_test_utils.int_exn diff1 "a__diff")
    (-1)
    "diff1 a__diff should be -1";
  assert_throws
    (Telemetry_test_utils.int_exn diff1)
    "a__prev"
    "not found"
    "diff1 a__prev should throw";

  (* different values, only show changed *)
  begin
    let diff1b = Telemetry.diff ~all:false current1 ~prev:prev1 in
    assert_throws
      (Telemetry_test_utils.int_exn diff1b)
      "a"
      "not found"
      "diff1b a should throw";
    Int_asserter.assert_equals
      (Telemetry_test_utils.int_exn diff1b "a__diff")
      (-1)
      "diff1b a__diff should be -1";
    assert_throws
      (Telemetry_test_utils.int_exn diff1b)
      "a__prev"
      "not found"
      "diff1b a__prev should throw"
  end;

  (* same values *)
  let current2 = Telemetry.create () |> Telemetry.int_ ~key:"b" ~value:1 in
  let prev2 = Telemetry.create () |> Telemetry.int_ ~key:"b" ~value:1 in
  begin
    let diff2 = Telemetry.diff ~all:true current2 ~prev:prev2 in
    Int_asserter.assert_equals
      (Telemetry_test_utils.int_exn diff2 "b")
      1
      "diff2 b should be 1";
    assert_throws
      (Telemetry_test_utils.int_exn diff2)
      "a__prev"
      "not found"
      "diff2 a__prev should throw";

    (* same value, only show changed *)
    let diff2b = Telemetry.diff ~all:false current2 ~prev:prev2 in
    assert_throws
      (Telemetry_test_utils.int_exn diff2b)
      "a"
      "not found"
      "diff2b a should throw";
    assert_throws
      (Telemetry_test_utils.int_exn diff2b)
      "a__prev"
      "not found"
      "diff2b a__prev should throw"
  end;

  (* nested object *)
  let current3 =
    Telemetry.create ()
    |> Telemetry.object_
         ~key:"o"
         ~value:
           (Telemetry.create ()
           |> Telemetry.int_ ~key:"a" ~value:1
           |> Telemetry.int_ ~key:"b" ~value:1)
  in
  let prev3 =
    Telemetry.create ()
    |> Telemetry.object_
         ~key:"o"
         ~value:
           (Telemetry.create ()
           |> Telemetry.int_ ~key:"a" ~value:2
           |> Telemetry.int_ ~key:"b" ~value:1)
  in
  begin
    let diff3 = Telemetry.diff ~all:true current3 ~prev:prev3 in
    Int_asserter.assert_equals
      (Telemetry_test_utils.int_exn diff3 "o.a")
      1
      "diff3 o.a should be 1";
    Int_asserter.assert_equals
      (Telemetry_test_utils.int_exn diff3 "o.a__diff")
      (-1)
      "diff3 o.a__diff should be -1";
    Int_asserter.assert_equals
      (Telemetry_test_utils.int_exn diff3 "o.b")
      1
      "diff3 o.b should be 1";
    assert_throws
      (Telemetry_test_utils.int_exn diff3)
      "o.b__prev"
      "not found"
      "diff3 o.b__prev should throw"
  end;

  (* nested object, only show different *)
  begin
    let diff3b = Telemetry.diff ~all:false current3 ~prev:prev3 in
    assert_throws
      (Telemetry_test_utils.int_exn diff3b)
      "o.a"
      "not found"
      "diff3b o.a should throw";
    Int_asserter.assert_equals
      (Telemetry_test_utils.int_exn diff3b "o.a__diff")
      (-1)
      "diff3 o.a__diff should be -1";
    assert_throws
      (Telemetry_test_utils.int_exn diff3b)
      "o.b"
      "not found"
      "diff3b o.b should throw";
    assert_throws
      (Telemetry_test_utils.int_exn diff3b)
      "o.b__diff"
      "not found"
      "diff3b o.b__diff should throw"
  end;

  (* prev absent, and current absent *)
  let current4 =
    Telemetry.create ()
    |> Telemetry.object_
         ~key:"o"
         ~value:(Telemetry.create () |> Telemetry.int_ ~key:"c" ~value:3)
  in
  let prev4 =
    Telemetry.create ()
    |> Telemetry.object_
         ~key:"p"
         ~value:(Telemetry.create () |> Telemetry.int_ ~key:"d" ~value:4)
  in
  begin
    let diff4 = Telemetry.diff ~all:true current4 ~prev:prev4 in
    Int_asserter.assert_equals
      (Telemetry_test_utils.int_exn diff4 "o.c")
      3
      "diff4 o.c should be 3";
    Bool_asserter.assert_equals
      (match Telemetry_test_utils.value_exn diff4 "o__prev" with
      | Hh_json.JSON_Null -> true
      | _ -> false)
      true
      "diff4 o__prev should be JSON_Null";
    assert_throws
      (Telemetry_test_utils.value_exn diff4)
      "o.c__prev"
      "not found"
      "diff4 o.c__prev should throw";
    Bool_asserter.assert_equals
      (match Telemetry_test_utils.value_exn diff4 "p" with
      | Hh_json.JSON_Null -> true
      | _ -> false)
      true
      "diff4 p should be JSON_Null";
    Int_asserter.assert_equals
      (Telemetry_test_utils.int_exn diff4 "p__prev.d__prev")
      4
      "diff4 p__prev.d__prev should be 4"
  end;
  true

let () =
  Unit_test.run_all
    [
      ("test ability to split namespaces", test_namespace_splitter);
      ("test ability to expand namespaces", test_expand_namespace);
      ("test strip namespace functions", test_strip_namespace);
      ("test telemetry_test functions", test_telemetry_test);
      ("test telemetry_diff", test_telemetry_diff);
    ]
