(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_core
open HoverService

module Test = Integration_test_base

let pos_at (line1, column1) (line2, column2) =
  Some (Pos.make_from_file_pos
    Relative_path.default
    (File_pos.of_line_column_offset line1 (column1 - 1) 0)
    (File_pos.of_line_column_offset line2 column2 0))

let class_members = "<?hh // strict
abstract class ClassMembers {
  public async function genDoStuff(): Awaitable<void> {}

  public string $public = 'public';
  protected string $protected = 'protected';
  private string $private = 'private';

  public static string $staticVar = 'staticVar';

  public abstract function abstractMethod(): string;

  public final function finalMethod(string $arg): void {}

  protected final static async function genLotsOfModifiers(): Awaitable<void> {}

  public async function exerciseClassMembers(): Awaitable<void> {
    await $this->genDoStuff();
//               ^18:18
    $this->public;
//         ^20:12
    $this->protected;
//         ^22:12
    $this->private;
//         ^24:12
    ClassMembers::$staticVar;
//                ^26:19
    $this->abstractMethod();
//         ^28:12
    $this->finalMethod(\"arg\");
//         ^30:12
    await ClassMembers::genLotsOfModifiers();
//        ^32:11        ^32:25
  }
}"

let class_members_cases = [
  ("class_members.php", 18, 18), [
    {
      snippet = "public async function genDoStuff(): Awaitable<void>";
      addendum = ["Full name: `ClassMembers::genDoStuff`"];
      pos = pos_at (18, 18) (18, 27);
    }
  ];
  ("class_members.php", 20, 12), [
    {
      snippet = "public string ClassMembers::public";
      addendum = [];
      pos = pos_at (20, 12) (20, 17);
    };
  ];
  ("class_members.php", 22, 12), [
    {
      snippet = "protected string ClassMembers::protected";
      addendum = [];
      pos = pos_at (22, 12) (22, 20);
    };
  ];
  ("class_members.php", 24, 12), [
    {
      snippet = "private string ClassMembers::private";
      addendum = [];
      pos = pos_at (24, 12) (24, 18);
    };
  ];
  ("class_members.php", 26, 19), [
    {
      snippet = "public static string ClassMembers::staticVar";
      addendum = [];
      pos = pos_at (26, 19) (26, 28);
    };
  ];
  ("class_members.php", 28, 12), [
    {
      snippet = "public abstract function abstractMethod(): string";
      addendum = ["Full name: `ClassMembers::abstractMethod`"];
      pos = pos_at (28, 12) (28, 25);
    }
  ];
  ("class_members.php", 30, 12), [
    {
      snippet = "public final function finalMethod(string $arg): void";
      addendum = ["Full name: `ClassMembers::finalMethod`"];
      pos = pos_at (30, 12) (30, 22);
    };
  ];
  ("class_members.php", 32, 11), [
    {
      snippet = "abstract class ClassMembers";
      addendum = [];
      pos = pos_at (32, 11) (32, 22);
    };
  ];
  ("class_members.php", 32, 25), [
    {
      snippet = "protected final static async\n\
                 function genLotsOfModifiers(): Awaitable<void>";
      addendum = ["Full name: `ClassMembers::genLotsOfModifiers`"];
      pos = pos_at (32, 25) (32, 42);
    };
  ];
]

let classname_call = "<?hh // strict
class ClassnameCall {
  static function foo(): int {
    return 0;
  }
}

function call_foo(): void {
  ClassnameCall::foo();
// ^9:4          ^9:18
}"

let classname_call_cases = [
  ("classname_call.php", 9, 4), [{
      snippet = "class ClassnameCall";
      addendum = [];
      pos = pos_at (9, 3) (9, 15);
    }];
  ("classname_call.php", 9, 18), [{
      snippet = "static function foo(): int";
      addendum = ["Full name: `ClassnameCall::foo`"];
      pos = pos_at (9, 18) (9, 20);
    }];
]

let chained_calls = "<?hh // strict
class ChainedCalls {
  public function foo(): this {
    return $this;
  }
}

function test(): void {
  $myItem = new ChainedCalls();
  $myItem
    ->foo()
    ->foo()
    ->foo();
//     ^13:8
}"

let chained_calls_cases = [
  ("chained_calls.php", 13, 8), [
    {
      snippet = "public function foo(): ChainedCalls";
      addendum = ["Full name: `ChainedCalls::foo`"];
      pos = pos_at (13, 7) (13, 9);
    };
  ];
]

let multiple_potential_types = "<?hh // strict
class C1 { public function foo(): int { return 5; } }
class C2 { public function foo(): string { return 's'; } }
function test_multiple_type(C1 $c1, C2 $c2, bool $cond): arraykey {
  $x = $cond ? $c1 : $c2;
  return $x->foo();
//        ^6:11^6:16
}"

let multiple_potential_types_cases = [
  ("multiple_potential_types.php", 6, 11), [{
      snippet = "(C1 | C2)";
      addendum = [];
      pos = None;
    }];
  ("multiple_potential_types.php", 6, 16), [
    {
      snippet = "((function(): string) | (function(): int))";
      addendum = [];
      pos = None;
    };
    {
      snippet = "((function(): string) | (function(): int))";
      addendum = [];
      pos = None;
    };
  ];
]

let classname_variable = "<?hh // strict
class ClassnameVariable {
  public static function foo(): void {}
}

function test_classname(): void {
  $cls = ClassnameVariable::class;
  $cls::foo();
// ^8:4  ^8:10
}"

let classname_variable_cases = [
  ("classname_variable.php", 8, 4), [{
      snippet = "classname<ClassnameVariable>";
      addendum = [];
      pos = pos_at (8, 3) (8, 6);
    }];

  ("classname_variable.php", 8, 10), [{
      snippet = "public static function foo(): void";
      addendum = ["Full name: `ClassnameVariable::foo`"];
      pos = pos_at (8, 9) (8, 11);
    }];
]

let docblock = "<?hh // strict

// Multiline
// function
// doc block.
function queryDocBlocks(): void {
  DocBlock::doStuff();
//^7:3     ^7:13
  queryDocBlocks();
//^9:3
  DocBlock::preserveIndentation();
//          ^11:13
  DocBlock::leadingStarsAndMDList();
//          ^13:13
  DocBlock::manyLineBreaks();
//          ^15:13
  $x = new DocBlockOnClassButNotConstructor();
//         ^17:12
}

function docblockReturn(): DocBlockBase {
//                         ^21:28
  $x = new DocBlockBase();
//         ^23:12
  return new DocBlockDerived();
//           ^25:14
}

/* Class doc block.
   This
   doc
   block
   has
   multiple
   lines. */
class DocBlock {
  /** Method doc block with double star. */
  public static function doStuff(): void {}

  /** Multiline doc block with
      a certain amount of
          indentation
      we want to preserve. */
  public static function preserveIndentation(): void {}

  /** Multiline doc block with
    * leading stars, as well as
    *   * a Markdown list!
    * and we'd really like to preserve the Markdown list while getting rid of
    * the other stars. */
  public static function leadingStarsAndMDList(): void {}

  // This method has many line breaks, which
  //
  // someone might use if they wanted
  //
  // to have separate paragraphs
  //
  // in Markdown.
  public static function manyLineBreaks(): void {}
}

/**
 * Class doc block for a class whose constructor doesn't have a doc block.
 */
final class DocBlockOnClassButNotConstructor {
  public function __construct() {}
}

/* DocBlockBase: class doc block. */
class DocBlockBase {
  /* DocBlockBase: constructor doc block. */
  public function __construct() {}
}

/* DocBlockDerived: extends a class with a constructor, but doesn't have one of
   its own. */
class DocBlockDerived extends DocBlockBase {}

// Line comments with breaks

// We don't want the line comment above to be part of this docblock.
// We do want both these lines though.
function line_comment_with_break(): void {}
//       84^:10

// This is another special case.
/** Only this should be part of the docblock. */
function two_comment_types(): void {}
//       ^89:10

// There are too many blank lines between this comment and what it's commenting
// on.


function too_many_blank_lines(): void {}
//       ^96:10

// For legacy reasons, we have to support a single linebreak between a docblock
// and the item the docblock is for.

function one_linebreak_is_okay(): void {}
//       ^102:10

/** A function with an HH_FIXME. */
/* HH_FIXME[4030] Missing return type hint. */
function hh_fixme() {}
//       ^107:10
"

let docblock_cases = [
  ("docblock.php", 7, 3), [
    {
      snippet = "class DocBlock";
      addendum = ["Class doc block.\n\
                   This\n\
                   doc\n\
                   block\n\
                   has\n\
                   multiple\n\
                   lines."];
       pos = pos_at (7, 3) (7, 10);
    }
  ];
  ("docblock.php", 7, 13), [
    {
      snippet = "public static function doStuff(): void";
      addendum = ["Method doc block with double star."; "Full name: `DocBlock::doStuff`"];
      pos = pos_at (7, 13) (7, 19);
    }
  ];
  ("docblock.php", 9, 3), [
    {
      snippet = "function queryDocBlocks(): void";
      addendum = ["Multiline\n\
                   function\n\
                   doc block."];
      pos = pos_at (9, 3) (9, 16);
    }
  ];
  ("docblock.php", 11, 13), [
    {
      snippet = "public static function preserveIndentation(): void";
      addendum = ["Multiline doc block with
a certain amount of
    indentation
we want to preserve."; "Full name: `DocBlock::preserveIndentation`"];
      pos = pos_at (11, 13) (11, 31);
    }
  ];
  ("docblock.php", 13, 13), [
    {
      snippet = "public static function leadingStarsAndMDList(): void";
      addendum = ["Multiline doc block with
leading stars, as well as
  * a Markdown list!
and we'd really like to preserve the Markdown list while getting rid of
the other stars."; "Full name: `DocBlock::leadingStarsAndMDList`"];
      pos = pos_at (13, 13) (13, 33);
    }
  ];
  ("docblock.php", 15, 13), [
    {
      snippet = "public static function manyLineBreaks(): void";
      addendum = [
        "This method has many line breaks, which\n\
         \n\
         someone might use if they wanted\n\
         \n\
         to have separate paragraphs\n\
         \n\
         in Markdown.";
      "Full name: `DocBlock::manyLineBreaks`"];
      pos = pos_at (15, 13) (15, 26);
    }
  ];
  ("docblock.php", 17, 12), [
    {
      snippet = "public function __construct(): _";
      addendum = [
        "Full name: `DocBlockOnClassButNotConstructor::__construct`";
      ];
      pos = pos_at (17, 8) (17, 45);
    }
  ];
  ("docblock.php", 21, 28), [
    {
      snippet = "DocBlockBase";
      addendum = ["DocBlockBase: class doc block."];
      pos = pos_at (21, 28) (21, 39);
    }
  ];
  ("docblock.php", 23, 12), [
    {
      snippet = "public function __construct(): _";
      addendum = [
        "DocBlockBase: constructor doc block.";
        "Full name: `DocBlockBase::__construct`";
      ];
      pos = pos_at (23, 8) (23, 25);
    }
  ];
  ("docblock.php", 25, 14), [
    {
      snippet = "public function __construct(): _";
      addendum = [
        "DocBlockBase: constructor doc block.";
        "Full name: `DocBlockBase::__construct`";
      ];
      pos = pos_at (25, 10) (25, 30);
    }
  ];
  ("docblock.php", 84, 10), [
    {
      snippet = "line_comment_with_break";
      addendum = [
        "We don't want the line comment above to be part of this docblock.\n\
        We do want both these lines though."
      ];
      pos = pos_at (84, 10) (84, 32);
    }
  ];
  ("docblock.php", 89, 10), [
    {
      snippet = "two_comment_types";
      addendum = ["Only this should be part of the docblock."];
      pos = pos_at (89, 10) (89, 26);
    }
  ];
  ("docblock.php", 96, 10), [
    {
      snippet = "too_many_blank_lines";
      addendum = [];
      pos = pos_at (96, 10) (96, 29);
    }
  ];
  ("docblock.php", 102, 10), [
    {
      snippet = "one_linebreak_is_okay";
      addendum = ["For legacy reasons, we have to support a single linebreak between a docblock\n\
                   and the item the docblock is for."];
      pos = pos_at (102, 10) (102, 30);
    }
  ];
  ("docblock.php", 107, 10), [
    {
      snippet = "hh_fixme";
      addendum = ["A function with an HH_FIXME."];
      pos = pos_at (107, 10) (107, 17);
    }
  ];
]

let special_cases = "<?hh // strict
function special_cases(): void {
  idx(array(1, 2, 3), 1);
//^3:3
}
"

let special_cases_cases = [
  ("special_cases.php", 3, 3), [
    {
      snippet = "\
function idx(
  ?KeyedContainer<int, ?int> $collection,
  ?int $index
): ?int";
      addendum = ["\n\
                    Index into the given KeyedContainer using the provided key.\n\
                    \n\
                    If the key doesn't exist, the key is `null`, or the collection is `null`,\n\
                    return the provided default value instead, or `null` if no default value was\n\
                    provided. If the key is `null`, the default value will be returned even if\n\
                    `null` is a valid key in the container.\n"];
      pos = pos_at (3, 3) (3, 5);
    }
  ]
]

let bounded_generic_fun = "<?hh // strict
abstract class Base {}
final class C extends Base {}
function bounded_generic_fun<T as Base>(T $x): void {
  $x;
//^5:3
  if ($x instanceof C) {
//    ^7:7
    $x;
//  ^9:5
  }
  $x;
//^12:3
}
"

let bounded_generic_fun_cases = [
  ("bounded_generic_fun.php", 5, 3), [{
    snippet = "T\nwhere T as Base";
    addendum = [];
    pos = pos_at (5, 3) (5, 4);
  }];
  ("bounded_generic_fun.php", 7, 7), [{
    snippet = "T\nwhere T as Base";
    addendum = [];
    pos = pos_at (7, 7) (7, 8);
  }];
  ("bounded_generic_fun.php", 9, 5), [{
    snippet = "C";
    addendum = [];
    pos = pos_at (9, 5) (9, 6);
  }];
  ("bounded_generic_fun.php", 12, 3), [{
    snippet = "(C | T)\nwhere T as Base";
    addendum = [];
    pos = pos_at (12, 3) (12, 4);
  }];
]

let files = [
  "class_members.php", class_members;
  "classname_call.php", classname_call;
  "chained_calls.php", chained_calls;
  "classname_variable.php", classname_variable;
  "docblock.php", docblock;
  "special_cases.php", special_cases;
  "bounded_generic_fun.php", bounded_generic_fun;
]

let cases =
  special_cases_cases
  @ docblock_cases
  @ class_members_cases
  @ classname_call_cases
  @ chained_calls_cases
  @ classname_variable_cases
  @ bounded_generic_fun_cases

let () =
  let env =
    Test.setup_server ()
      ~hhi_files:(Hhi.get_raw_hhi_contents () |> Array.to_list)
  in
  let env = Test.setup_disk env files in

  Test.assert_no_errors env;

  let failed_cases =
    List.filter_map cases ~f:begin fun ((file, line, col), expectedHover) ->
      let list_to_string hover_list =
        let string_list = hover_list |> List.map ~f:HoverService.string_of_result in
        let inner = match string_list |> List.reduce ~f:(fun a b -> a ^ "; " ^ b) with
          | None -> ""
          | Some s -> s
        in
        Printf.sprintf "%s:%d:%d: [%s]" file line col inner
      in
      let fn = ServerCommandTypes.FileName ("/" ^ file) in
      let hover = ServerHover.go env (fn, line, col) in
      let expected = list_to_string expectedHover in
      let actual = list_to_string hover in
      if expected <> actual then Some (expected, actual) else None
  end in
  match failed_cases with
  | [] -> ()
  | cases ->
    let display_case (expected, actual) =
      Printf.sprintf "Expected:\n%s\nGot:\n%s" expected actual
    in
    Test.fail @@ String.concat "\n\n" (List.map ~f:display_case cases)
