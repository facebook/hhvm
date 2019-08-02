open Asserter

let test_ns_split name assert_left assert_right =
  let (left, right) = Utils.split_ns_from_name name in
  String_asserter.assert_equals left assert_left "Namespace is wrong";
  String_asserter.assert_equals right assert_right "Namespace is wrong"

let test_namespace_splitter () =
  test_ns_split "HH\\Lib\\Str\\Format" "HH\\Lib\\Str\\" "Format";
  test_ns_split "NameWithoutANamespace" "\\" "NameWithoutANamespace";
  test_ns_split "HH\\Lib\\Str\\" "HH\\Lib\\Str\\" "";
  test_ns_split
    "\\HH\\Lib\\Hellothisisafunction"
    "\\HH\\Lib\\"
    "Hellothisisafunction";
  true

let () =
  Unit_test.run_all
    [("test ability to split namespaces", test_namespace_splitter)]
