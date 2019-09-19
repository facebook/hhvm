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

let assert_cm_split str expected : unit =
  Printf.printf "Testing [%s]\n" str;
  let r = Utils.split_class_from_method str in
  let success =
    match (r, expected) with
    | (None, None) -> true
    | (Some (a, b), Some (c, d)) -> a = c && b = d
    | _ -> false
  in
  ( if not success then
    let msg =
      Printf.sprintf "ASSERTION FAILURE: [%s] did not split correctly" str
    in
    failwith msg );
  ()

let test_class_meth_splitter () =
  assert_cm_split "A::B" (Some ("A", "B"));
  assert_cm_split
    "AReallyLongName::AnotherLongName"
    (Some ("AReallyLongName", "AnotherLongName"));
  assert_cm_split "::B" None;
  assert_cm_split "A::" None;
  assert_cm_split "::" None;
  assert_cm_split "Justsomerandomtext" None;
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

let () =
  Unit_test.run_all
    [
      ("test ability to split namespaces", test_namespace_splitter);
      ("test ability to split class::meth", test_class_meth_splitter);
      ("test ability to expand namespaces", test_expand_namespace);
      ("test strip namespace functions", test_strip_namespace);
    ]
