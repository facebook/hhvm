(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Prelude = struct
  let decl_function name = Printf.sprintf "function %s(): void {}" name

  let decl_stringish name =
    Printf.sprintf
      "
    class %s {
      public function __toString(): string { return ''; }
    }
    "
      name

  let decl_xhp name = Printf.sprintf "class :%s implements XHPChild {}" name

  let decl_enum_class name =
    Printf.sprintf
      "
  enum class %s : nonnull {
    float A = 3.141;
    (float, float) B = tuple(0.0, 0.0);
  }"
      name

  let decl_enum name =
    Printf.sprintf "
  enum %s : string {
    A = 'A';
    B = 'B';
  }" name

  let decl_interface name =
    Printf.sprintf
      "
  interface %s {}

  class InstanceOf%s implements %s {}
  "
      name
      name
      name

  let decl_class name = Printf.sprintf "class %s {}" name

  let decl_reified_class name = Printf.sprintf "class %s<reify T> {}" name

  let decl_trait name =
    Printf.sprintf
      "
  trait %s {}

  class Uses%s {
    use %s;
  }"
      name
      name
      name

  let decl_abstract_final_class name =
    Printf.sprintf "abstract final class %s {}" name
end

module TypeSpec = struct
  type t = {
    preludes: String.Set.t;
    examples: String.Set.t;
  }

  type kind =
    | Int
    | Null
    | Bool
    | String
    | Float
    | Tuple
    | Shape
    | Resource
    | Lambda
    | Function of string
    | FunctionRef of string
    | StringishObj of string
    | XHP of string
    | MemberOf of string
    | LabelOf of string
    | Enum of string
    | Interface of string
    | Class of string * string option
    | Vec
    | Dict
    | Keyset
    | Trait of string
    | Uninhabited of string
    | Nullable of kind
    | Awaitable of kind
    | Classname of string

  let build types =
    let rec build_for_type = function
      | Int -> ([], ["0"; "1"])
      | Null -> ([], ["null"])
      | Bool -> ([], ["true"; "false"])
      | String -> ([], ["'hello world'"; "''"])
      | Float -> ([], ["0.0"; "3.14"])
      | Tuple -> ([], ["tuple(0, 0)"; "tuple(1, 2, 3)"])
      | Shape -> ([], ["shape()"; "shape('x' => 10)"])
      | Resource -> ([], ["imagecreate(10, 10)"])
      | Lambda -> ([], ["() ==> {}"])
      | Function name ->
        ([Prelude.decl_function name], [Printf.sprintf "vec['%s']" name])
      | FunctionRef name ->
        ([Prelude.decl_function name], [Printf.sprintf "%s<>" name])
      | StringishObj name ->
        ([Prelude.decl_stringish name], [Printf.sprintf "new %s()" name])
      | XHP name ->
        let name = name ^ "-xhp" in
        ([Prelude.decl_xhp name], [Printf.sprintf "<%s/>" name])
      | MemberOf name ->
        ( [Prelude.decl_enum_class name],
          [Printf.sprintf "%s::A" name; Printf.sprintf "%s::B" name] )
      | LabelOf name ->
        ([Prelude.decl_enum_class name], ["#A"; Printf.sprintf "%s#B" name])
      | Enum name ->
        ([Prelude.decl_enum name], [Printf.sprintf "%s::A" name; "'B'"])
      | Interface name ->
        ( [Prelude.decl_interface name],
          [Printf.sprintf "new InstanceOf%s()" name] )
      | Trait name -> ([Prelude.decl_trait name], [])
      | Class (name, None) ->
        ([Prelude.decl_class name], [Printf.sprintf "new %s()" name])
      | Class (name, Some tyarg) ->
        ( [Prelude.decl_reified_class name],
          [Printf.sprintf "new %s<%s>()" name tyarg] )
      | Vec -> ([], ["vec[]"])
      | Dict -> ([], ["dict[]"])
      | Keyset -> ([], ["keyset[]"])
      | Uninhabited name -> ([Prelude.decl_abstract_final_class name], [])
      | Nullable value ->
        let (decls, values) = build_for_type value in
        (decls, "null" :: values)
      | Awaitable value ->
        let (decls, values) = build_for_type value in
        (decls, List.map ~f:(Printf.sprintf "async { return %s; }") values)
      | Classname name ->
        ([Prelude.decl_class name], [Printf.sprintf "%s::class" name])
    in
    List.fold
      types
      ~init:{ preludes = String.Set.empty; examples = String.Set.empty }
      ~f:(fun spec typ ->
        let (preludes, examples) = build_for_type typ in
        {
          preludes = Set.union spec.preludes (String.Set.of_list preludes);
          examples = Set.union spec.examples (String.Set.of_list examples);
        })
end

module TestInstance = struct
  module PositionedTree =
    Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

  type t = {
    test_name: string;
    disable_test: bool;
    config: (string * TypeSpec.t) list list;
  }

  let format_src src =
    src
    |> Full_fidelity_source_text.make Relative_path.default
    |> PositionedTree.make
    |> Libhackfmt.format_tree

  let declare_checker name values : string =
    Printf.sprintf
      "
class Check%s<T as %s> extends BaseCheck {
  const type T = %s;
  const string NAME = '%s';

  <<__LateInit>>
  private %s $field;
  <<__LateInit>>
  private T $generic_field;

  protected static function funcParam(%s $c): void {}

  protected static function funcReturn(mixed $c): %s {
    return $c;
  }

  protected static function funcGenericParam<Tx as %s>(Tx $c): void {}

  protected static function funcGenericReturn<Tx as %s>(mixed $c): Tx {
    return $c;
  }

  protected static function propertyCheck(%s $val): void {
    $instance = new self();
    $instance->field = $val;
    $instance->generic_field = $val;
  }

  protected static function values(): vec<%s> {
    return vec[%s];
  }
}"
      name
      name
      name
      name
      name
      name
      name
      name
      name
      name
      name
      values

  let make_source config =
    let ((preludes, decls, checks), expects) =
      List.fold_mapi
        config
        ~init:(String.Set.empty, [], [])
        ~f:(fun idx (preludes, decls, checks) specs ->
          let (preludes, types, examples) =
            List.fold
              ~init:(preludes, [], String.Set.empty)
              ~f:(fun (preludes, types, examples) (type_str, spec) ->
                ( Set.union preludes spec.TypeSpec.preludes,
                  type_str :: types,
                  Set.union examples spec.TypeSpec.examples ))
              specs
          in
          let union_decl = String.concat ~sep:"|" types in
          let values = String.concat ~sep:"," (Set.elements examples) in
          let name = "CT" ^ string_of_int idx in
          let alias_name = "AliasCT" ^ string_of_int idx in
          let decls =
            Printf.sprintf
              "case type %s = %s;
              type %s = %s;

  %s"
              name
              union_decl
              alias_name
              name
              (declare_checker alias_name values)
            :: decls
          in
          let name = alias_name in
          let checks = Printf.sprintf "Check%s::run();" name :: checks in
          ((preludes, decls, checks), name ^ " Ok"))
    in
    let prelude = String.concat ~sep:"\n" (Set.elements preludes) in
    let decl = String.concat ~sep:"\n" (List.rev decls) in
    let check = String.concat ~sep:"\n" (List.rev checks) in
    let src =
      Printf.sprintf
        "<?hh
  /**
   * THIS FILE IS @%s; DO NOT EDIT IT
   * To regenerate this file, run
   *
   *   buck run //hphp/hack/test:gen_case_type_tests
   **/

  <<file: __EnableUnstableFeatures('case_types')>>

  %s

  abstract class BaseCheck {
    abstract const type T;
    abstract const string NAME;
    abstract protected static function values(): vec<this::T>;
    abstract protected static function funcParam(this::T $c): void;
    abstract protected static function funcReturn(mixed $c): this::T;
    abstract protected static function funcGenericParam<Tx as this::T>(Tx $c): void;
    abstract protected static function funcGenericReturn<Tx as this::T>(mixed $c): Tx;
    abstract protected static function propertyCheck(this::T $val): void;

    public static function run(): void {
      foreach(static::values() as $val) {
        // Param Checks
        static::funcParam($val);
        static::funcGenericParam($val);

        // Return Checks
        static::funcReturn($val);
        static::funcGenericReturn($val);

        // Property Checks
        static::propertyCheck($val);
      }
      echo (static::NAME .' Ok' . PHP_EOL);
    }
  }

  %s

  <<__EntryPoint>>
  function main(): void {
    %s
  }"
        "generated"
        prelude
        decl
        check
    in
    (format_src src, String.concat ~sep:"\n" expects)

  let generate { test_name; disable_test; config } =
    let (src, expects) = make_source config in
    let write_file filename src =
      let file = Out_channel.create filename in
      Printf.fprintf file "%s" src;
      Out_channel.close file
    in
    let filename =
      if disable_test then
        test_name ^ ".disable"
      else
        test_name
    in
    write_file filename src;
    let expect_filename = test_name ^ ".expect" in
    write_file expect_filename expects
end

let type_to_specs =
  let open TypeSpec in
  let config =
    String.Map.of_alist_exn
      [
        ("int", [Int]);
        ("null", [Null]);
        ("bool", [Bool]);
        ("string", [String]);
        ("float", [Float]);
        ("(mixed, mixed)", [Tuple]);
        ("shape(...)", [Shape]);
        ("void", [Null]);
        ("resource", [Resource]);
        ("noreturn", []);
        ("nothing", []);
        ( "(function(): void)",
          [Lambda; Function "my_func"; FunctionRef "my_func"] );
        ("HH\\FunctionRef<(function(): void)>", [FunctionRef "my_func"]);
        (* ("supportdyn<(function(): void)>",
                 [Lambda; Function "my_func"; FunctionRef "my_func"]
              ); *)
        ("num", [Int; Float]);
        ("arraykey", [Int; String]);
        ("nonnull", [Bool; String; Int]);
        ("mixed", [Null; Bool; String; Int]);
        ("dynamic", [Null; Bool; Shape]);
        ("Stringish", [String; StringishObj "StringishObj"]);
        ("XHPChild", [String; Int; XHP "my"]);
        ("HH\\MemberOf<EC, float>", [MemberOf "EC"]);
        ("HH\\EnumClass\\Label<EC, float>", [LabelOf "EC"]);
        ("MyEnum", [Enum "MyEnum"]);
        ("I", [Interface "I"]);
        ("AClass", [Class ("AClass", None)]);
        ("ReifiedClass<null>", [Class ("ReifiedClass", Some "null")]);
        ("Traversable<mixed>", [Vec; Keyset; Dict]);
        ("KeyedTraversable<arraykey, mixed>", [Vec; Keyset; Dict]);
        ("Container<mixed>", [Vec; Keyset]);
        ("KeyedContainer<arraykey, mixed>", [Vec; Dict]);
        ("dict<arraykey, mixed>", [Dict]);
        ("vec<mixed>", [Vec]);
        ("keyset<arraykey>", [Keyset]);
        ("vec_or_dict<string>", [Vec; Dict]);
        ("HH\\AnyArray<arraykey, mixed>", [Vec; Dict; Keyset]);
        ("?bool", [Nullable Bool]);
        ("Awaitable<num>", [Awaitable Int; Awaitable Float]);
        ("MyTrait", [Trait "MyTrait"]);
        ("AbsFinal", [Uninhabited "AbsFinal"]);
      ]
  in
  Map.map ~f:TypeSpec.build config

let invalid_types_to_specs =
  let config = String.Map.of_alist_exn [("NonExistent", [])] in
  Map.map ~f:TypeSpec.build config
(*
  Other types to consider testing:
  | Type_access
  | Newtype
  | Generator
  | AsyncGenerator
  | FormatString
  | Throwable
  | IDisposable
  | Callable
*)

let () =
  let config = Map.to_alist type_to_specs in
  let (skipped_tests, good_tests) =
    List.cartesian_product config config
    |> List.filter_map ~f:(fun ((type1, spec1), (type2, spec2)) ->
           if String.compare type1 type2 <= 0 then
             let ok =
               SSet.of_list
                 [
                   "(function(): void)";
                   "HH\\FunctionRef<(function(): void)>";
                   "MyEnum";
                   "arraykey";
                   "dynamic";
                   "int";
                   "mixed";
                   "nonnull";
                   "noreturn";
                   "nothing";
                   "string";
                   "void";
                   "null";
                 ]
             in
             let disable_test =
               match (type1, type2) with
               | (x, "MyEnum")
               | ("MyEnum", x) ->
                 not @@ SSet.mem x ok
               | _ -> false
             in
             Some (disable_test, [(type1, spec1); (type2, spec2)])
           else
             None)
    |> List.partition_map ~f:(fun (disable_test, specs) ->
           if disable_test then
             Either.First specs
           else
             Either.Second specs)
  in
  let tests =
    Map.to_alist invalid_types_to_specs
    |> List.mapi ~f:(fun idx spec ->
           TestInstance.
             {
               test_name =
                 Printf.sprintf
                   "hphp/test/slow/case-types/generated-fuzz-test-invalid-%d.php"
                   (idx + 1);
               disable_test = true;
               config =
                 List.cartesian_product config [spec]
                 |> List.map ~f:(fun (spec1, spec2) -> [spec1; spec2]);
             })
  in
  let tests =
    tests
    @ List.mapi
        ~f:(fun idx test ->
          TestInstance.
            {
              test_name =
                Printf.sprintf
                  "hphp/test/slow/case-types/generated-fuzz-test-broken-alias-%d.php"
                  (idx + 1);
              disable_test = true;
              config = [test];
            })
        skipped_tests
  in
  let tests =
    TestInstance.
      {
        test_name = "hphp/test/slow/case-types/generated-fuzz-test.php";
        disable_test = false;
        config = good_tests;
      }
    :: tests
  in

  List.iter ~f:TestInstance.generate tests
