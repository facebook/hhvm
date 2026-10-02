(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let package ~name ~include_paths =
  {
    Package.name = (Pos.none, name);
    includes = [];
    soft_includes = [];
    include_paths = List.map include_paths ~f:(fun p -> (Pos.none, p));
    enable_strict_isolation = false;
    allow_deployed_packages_checking = false;
    is_implicit = false;
  }

let ctx =
  let package_info =
    List.fold
      [
        package ~name:"foo" ~include_paths:["foo/"];
        package ~name:"bar" ~include_paths:["bar/"];
      ]
      ~init:Package_info.empty
      ~f:Package_info.add_package
  in
  let tcopt = Global_options.default in
  let popt = { tcopt.Global_options.po with Parser_options.package_info } in
  Provider_context.empty_for_tool
    ~popt
    ~tcopt:{ tcopt with Global_options.po = popt }
    ~backend:Provider_backend.Shared_memory
    ~deps_mode:(Typing_deps_mode.InMemoryMode None)

(* foo/a.php is in package foo unless [content] overrides it. *)
let resolves_to ~package ~overridden content =
  let (pkg, is_overridden) =
    Package_provider.get_package_with_override_for_file_no_env
      ctx
      ~path:"foo/a.php"
      ~content
  in
  Option.equal String.equal (Option.map pkg ~f:Package.get_package_name) package
  && Bool.equal is_overridden overridden

let test_no_mention () =
  resolves_to ~package:(Some "foo") ~overridden:false "<?hh\nclass A {}\n"

let test_file_attribute () =
  resolves_to
    ~package:(Some "bar")
    ~overridden:true
    "<?hh\n<<file: __PackageOverride('bar')>>\nclass A {}\n"

let test_file_attribute_among_others () =
  resolves_to
    ~package:(Some "bar")
    ~overridden:true
    "<?hh\n<<file:\n  __EnableUnstableFeatures('union_intersection_type_hints'),\n  __PackageOverride('bar'),\n>>\nclass A {}\n"

let test_unknown_package () =
  resolves_to
    ~package:None
    ~overridden:true
    "<?hh\n<<file: __PackageOverride('baz')>>\nclass A {}\n"

let test_doc_comment () =
  resolves_to
    ~package:(Some "foo")
    ~overridden:false
    "<?hh\n/**\n * Stays in foo: no __PackageOverride('bar') here.\n */\nclass A {}\n"

let test_line_comment () =
  resolves_to
    ~package:(Some "foo")
    ~overridden:false
    "<?hh\n// @lint-ignore Uses a trait with __PackageOverride('bar')\nclass A {}\n"

let test_string_literal () =
  resolves_to
    ~package:(Some "foo")
    ~overridden:false
    "<?hh\nconst string ATTR = \"<<file: __PackageOverride('bar')>>\";\n"

let test_mention_before_the_attribute () =
  resolves_to
    ~package:(Some "bar")
    ~overridden:true
    "<?hh\n// __PackageOverride('foo') would be redundant here.\n<<file: __PackageOverride('bar')>>\nclass A {}\n"

let test_class_attribute () =
  resolves_to
    ~package:(Some "foo")
    ~overridden:false
    "<?hh\n<<__PackageOverride('bar')>>\nclass A {}\n"

let test_malformed_arguments () =
  List.for_all
    [
      "__PackageOverride";
      "__PackageOverride(1)";
      "__PackageOverride('bar', 'baz')";
    ]
    ~f:(fun attribute ->
      resolves_to
        ~package:(Some "foo")
        ~overridden:false
        (Printf.sprintf "<?hh\n<<file: %s>>\nclass A {}\n" attribute))

let test_repeated_override () =
  resolves_to
    ~package:(Some "bar")
    ~overridden:true
    "<?hh\n<<file: __PackageOverride('foo'), __PackageOverride('bar')>>\nclass A {}\n"

let test_override_in_an_earlier_block () =
  resolves_to
    ~package:(Some "bar")
    ~overridden:true
    "<?hh\n<<file: __PackageOverride('bar')>>\n<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>\nclass A {}\n"

let test_file_without_definitions () =
  resolves_to
    ~package:(Some "foo")
    ~overridden:false
    "<?hh\n<<file: __PackageOverride('bar')>>\n"

let () =
  Unit_test.run_all
    [
      ("a file with no mention keeps its package", test_no_mention);
      ("the file attribute overrides", test_file_attribute);
      ("an attribute among others overrides", test_file_attribute_among_others);
      ("an unknown package still overrides", test_unknown_package);
      ("a doc comment does not override", test_doc_comment);
      ("a line comment does not override", test_line_comment);
      ("a string literal does not override", test_string_literal);
      ( "a mention before the attribute does not shadow it",
        test_mention_before_the_attribute );
      ("a class attribute does not override", test_class_attribute);
      ( "an attribute without one string argument does not override",
        test_malformed_arguments );
      ("the last repeated override wins", test_repeated_override);
      ( "an override in an earlier block holds",
        test_override_in_an_earlier_block );
      ( "a file that defines nothing keeps its package",
        test_file_without_definitions );
    ]
