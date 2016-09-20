(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

type code_extent_test = {
  name: string;
  source: string;
  result: string;
  test_function: string -> string;
}

let dummy_path = Relative_path.default

let get_first_method_as_string contents =
  let {Parser_hack.ast; _} =
    Parser_hack.program_with_default_popt dummy_path contents in
  let methods = Ast_utils.get_methods ast in
  let method_ = List.hd_exn methods in
  let extent = Ast_code_extent.source_extent_method_
    dummy_path
    contents
    method_ in
  Ast_code_extent.lexing_slice_to_string extent contents

let get_first_typeConst_as_string contents =
  let {Parser_hack.ast; _} =
    Parser_hack.program_with_default_popt dummy_path contents in
  let typeconsts = Ast_utils.get_typeConsts ast in
  let typeconst = List.hd_exn typeconsts in
  let extent = Ast_code_extent.source_extent_typeConst contents typeconst in
  Ast_code_extent.lexing_slice_to_string extent contents

let get_first_classUse_as_string contents =
  let {Parser_hack.ast; _} =
    Parser_hack.program_with_default_popt dummy_path contents in
  let classUses = Ast_utils.get_classUses ast in
  let classUse = List.hd_exn classUses in
  let extent = Ast_code_extent.source_extent_classUse contents classUse in
  Ast_code_extent.lexing_slice_to_string extent contents

let get_first_class_header_as_string contents =
  let {Parser_hack.ast; _} =
    Parser_hack.program_with_default_popt dummy_path contents in
  let classes = Ast_utils.get_classes ast in
  let class_ = List.hd_exn classes in
  let extent = Ast_code_extent.source_extent_class_header
    contents
    class_ in
  Ast_code_extent.lexing_slice_to_string extent contents

let get_first_class_as_string contents =
  let {Parser_hack.ast; _} =
    Parser_hack.program_with_default_popt dummy_path contents in
  let classes = Ast_utils.get_classes ast in
  let class_ = List.hd_exn classes in
  let extent = Ast_code_extent.source_extent_class_
    dummy_path
    contents
    class_ in
  Ast_code_extent.lexing_slice_to_string extent contents

let test_data = [
  {
    name = "test_empty_method";
    source = String.concat "\n" [
      "<?hh";
      "class Foo {";
      "  public async function genFoo(): Awaitable<void> {}";
      "}";
    ];
    result = String.concat "\n" [
      "public async function genFoo(): Awaitable<void> {}";
    ];
    test_function = get_first_method_as_string;
  };
  {
    name = "test_method_with_if";
    source = String.concat "\n" [
      "<?hh";
      "class Foo {";
      "  public async function genFoo(): Awaitable<void> {";
      "    $x = 1; if ($x === 1) { return $x; } return 0;";
      "  }";
      "}";
    ];
    result = String.concat "\n" [
      "public async function genFoo(): Awaitable<void> {";
      "    $x = 1; if ($x === 1) { return $x; } return 0;";
      "  }";
    ];
    test_function = get_first_method_as_string;
  };
  {
    name = "test_typeConst_simple";
    source = String.concat "\n" [
      "<?hh";
      "class Foo {";
      "  const type TType = FooType;";
      "";
      "  public async function genFoo(): Awaitable<void> {";
      "    $x = 1; if ($x === 1) { return $x; } return 0;";
      "  }";
      "}";
    ];
    result = String.concat "\n" [
      "const type TType = FooType;";
    ];
    test_function = get_first_typeConst_as_string;
  };
  {
    name = "test_typeConst_with_generics";
    source = String.concat "\n" [
      "<?hh";
      "class Foo {";
      "  const type TType = FooType<int>;";
      "  const type TVal = FooVal;";
      "";
      "  public async function genFoo(): Awaitable<void> {";
      "    $x = 1; if ($x === 1) { return $x; } return 0;";
      "  }";
      "}";
    ];
    result = String.concat "\n" [
      "const type TType = FooType<int>;";
    ];
    test_function = get_first_typeConst_as_string;
  };
  {
    name = "test_typeConst_as_shape";
    source = String.concat "\n" [
      "<?hh";
      "class Foo {";
      "  const type TType = shape(";
      "    'shape_field_1' => int,";
      "    'shape_field_2' => string,";
      "  );";
      "";
      "  public async function genFoo(): Awaitable<void> {";
      "    $x = 1; if ($x === 1) { return $x; } return 0;";
      "  }";
      "}";
    ];
    result = String.concat "\n" [
      "const type TType = shape(";
      "    'shape_field_1' => int,";
      "    'shape_field_2' => string,";
      "  );";
    ];
    test_function = get_first_typeConst_as_string;
  };
  {
    name = "test_classUse_simple";
    source = String.concat "\n" [
      "<?hh";
      "class Foo {";
      "  use TFoo;";
      "  use TFoo2;";
      "";
      "  const type TType = shape(";
      "    'shape_field_1' => int,";
      "    'shape_field_2' => string,";
      "  );";
      "";
      "  public async function genFoo(): Awaitable<void> {";
      "    $x = 1; if ($x === 1) { return $x; } return 0;";
      "  }";
      "}";
    ];
    result = String.concat "\n" [
      "use TFoo;";
    ];
    test_function = get_first_classUse_as_string;
  };
  {
    name = "test_classUse_generics";
    source = String.concat "\n" [
      "<?hh";
      "class Foo {";
      "";
      "  const type TType = shape(";
      "    'shape_field_1' => int,";
      "    'shape_field_2' => string,";
      "  );";
      "";
      "  use TFoo<self::TType>;";
      "  use TFoo2;";
      "";
      "  public async function genFoo(): Awaitable<void> {";
      "    $x = 1; if ($x === 1) { return $x; } return 0;";
      "  }";
      "}";
    ];
    result = String.concat "\n" [
      "use TFoo<self::TType>;";
    ];
    test_function = get_first_classUse_as_string;
  };
  {
    name = "test_class_header_extends_implements";
    source = String.concat "\n" [
      "<?hh";
      "class Foo extends FooBase<string>";
      "  implements IFoo {";
      "";
      "  const type TType = shape(";
      "    'shape_field_1' => int,";
      "    'shape_field_2' => string,";
      "  );";
      "";
      "  use TFoo<self::TType>;";
      "  use TFoo2;";
      "";
      "  public async function genFoo(): Awaitable<void> {";
      "    $x = 1; if ($x === 1) { return $x; } return 0;";
      "  }";
      "}";
      "";
      "class FooBase<T> {}";
      "interface IFoo {}";
    ];
    result = String.concat "\n" [
      "class Foo extends FooBase<string>";
      "  implements IFoo {";
    ];
    test_function = get_first_class_header_as_string
  };
  {
    name = "test_class_header_extends_implements";
    source = String.concat "\n" [
      "<?hh";
      "class Foo extends FooBase<string> {";
      "}";
    ];
    result = String.concat "\n" [
      "class Foo extends FooBase<string> {";
      "}";
    ];
    test_function = get_first_class_as_string
  };
]

let driver test () =
  OUnit.assert_equal test.result (test.test_function test.source)

let test_suite =
  (OUnit.(>:::))
    "Ast_code_extent_suite"
    (List.map test_data ~f:(fun data ->
      OUnit.(>::) data.name (driver data)
    ))

let main () =
  let _handle = SharedMem.(init GlobalConfig.default_sharedmem_config) in
  OUnit.run_test_tt_main test_suite

let _ = main ()
