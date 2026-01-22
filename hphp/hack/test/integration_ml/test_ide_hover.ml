(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_prelude
open HoverService
module Test = Integration_test_base

let pos_at (line1, column1) (line2, column2) =
  Some
    (Pos.make_from_lnum_bol_offset
       ~pos_file:Relative_path.default
       ~pos_start:(line1, 0, column1 - 1)
       ~pos_end:(line2, 0, column2))

let class_members =
  "<?hh // strict
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
    $this->calculateDistance(5.0, -6.0, 12.0, 1.0);
//                         ^34:28
  }

  /** Another method doc block */
  public function calculateDistance(
    float $originalPositionX,
    float $finalPositionX,
    float $originalPositionY,
    float $finalPositionY,
  ): float {
    return sqrt(
      (float)pow($originalPositionX - $finalPositionX, 2) +
        (float)pow($originalPositionY - $finalPositionY, 2),
    );
  }
}
"

let class_members_cases =
  [
    ( ("class_members.php", 18, 18),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassMembers`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ("hack", "public async function genDoStuff(): Awaitable<void>");
            ];
          addendum = [];
          pos = pos_at (18, 18) (18, 27);
        };
      ] );
    ( ("class_members.php", 20, 12),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassMembers`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public string $public");
            ];
          addendum = [];
          pos = pos_at (20, 12) (20, 17);
        };
      ] );
    ( ("class_members.php", 22, 12),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassMembers`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "protected string $protected");
            ];
          addendum = [];
          pos = pos_at (22, 12) (22, 20);
        };
      ] );
    ( ("class_members.php", 24, 12),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassMembers`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "private string $private");
            ];
          addendum = [];
          pos = pos_at (24, 12) (24, 18);
        };
      ] );
    ( ("class_members.php", 26, 19),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassMembers`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public static string $staticVar");
            ];
          addendum = [];
          pos = pos_at (26, 19) (26, 28);
        };
      ] );
    ( ("class_members.php", 28, 12),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassMembers`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ("hack", "public abstract function abstractMethod(): string");
            ];
          addendum = [];
          pos = pos_at (28, 12) (28, 25);
        };
      ] );
    ( ("class_members.php", 30, 12),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassMembers`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ("hack", "public final function finalMethod(string $arg): void");
            ];
          addendum = [];
          pos = pos_at (30, 12) (30, 22);
        };
      ] );
    ( ("class_members.php", 32, 11),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "abstract class ClassMembers")];
          addendum = [];
          pos = pos_at (32, 11) (32, 22);
        };
      ] );
    ( ("class_members.php", 32, 25),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassMembers`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ( "hack",
                  "protected final static async
function genLotsOfModifiers(): Awaitable<void>"
                );
            ];
          addendum = [];
          pos = pos_at (32, 25) (32, 42);
        };
      ] );
    ( ("class_members.php", 34, 28),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassMembers`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ( "hack",
                  "public function calculateDistance(
  float $originalPositionX,
  float $finalPositionX,
  float $originalPositionY,
  float $finalPositionY
): float"
                );
            ];
          addendum = [Lsp.MarkedString "Another method doc block"];
          pos = pos_at (34, 12) (34, 28);
        };
      ] );
  ]

let classname_call =
  "<?hh // strict
class ClassnameCall {
  public static function foo(): int {
    return 0;
  }
}

function call_foo(): void {
  ClassnameCall::foo();
// ^9:4          ^9:18
}"

let classname_call_cases =
  [
    ( ("classname_call.php", 9, 4),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "class ClassnameCall")];
          addendum = [];
          pos = pos_at (9, 3) (9, 15);
        };
      ] );
    ( ("classname_call.php", 9, 18),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassnameCall`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public static function foo(): int");
            ];
          addendum = [];
          pos = pos_at (9, 18) (9, 20);
        };
      ] );
  ]

let chained_calls =
  "<?hh // strict
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

let chained_calls_cases =
  [
    ( ("chained_calls.php", 13, 8),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ChainedCalls`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function foo(): this");
              Lsp.MarkedString "---";
              Lsp.MarkedString "Instantiation:";
              Lsp.MarkedCode ("hack", "  this = ChainedCalls;");
            ];
          addendum = [];
          pos = pos_at (13, 7) (13, 9);
        };
      ] );
  ]

let multiple_potential_types =
  "<?hh // strict
class C1 { public function foo(): int { return 5; } }
class C2 { public function foo(): string { return 's'; } }
function test_multiple_type(C1 $c1, C2 $c2, bool $cond): arraykey {
  $x = $cond ? $c1 : $c2;
  return $x->foo();
//        ^6:11^6:16
}"

let multiple_potential_types_cases =
  [
    ( ("multiple_potential_types.php", 6, 11),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "(C1 | C2)")];
          addendum = [];
          pos = None;
        };
      ] );
    ( ("multiple_potential_types.php", 6, 16),
      [
        {
          snippet =
            [
              Lsp.MarkedCode
                ("hack", "((function(): string) | (function(): int))");
            ];
          addendum = [];
          pos = None;
        };
        {
          snippet =
            [
              Lsp.MarkedCode
                ("hack", "((function(): string) | (function(): int))");
            ];
          addendum = [];
          pos = None;
        };
      ] );
  ]

let classname_variable =
  "<?hh // strict
class ClassnameVariable {
  public static function foo(): void {}
}

function test_classname(): void {
  $cls = ClassnameVariable::class;
  $cls::foo();
// ^8:4  ^8:10
}"

let classname_variable_cases =
  [
    ( ("classname_variable.php", 8, 4),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "class<ClassnameVariable>")];
          addendum = [];
          pos = pos_at (8, 3) (8, 6);
        };
      ] );
    ( ("classname_variable.php", 8, 10),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `ClassnameVariable`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public static function foo(): void");
            ];
          addendum = [];
          pos = pos_at (8, 9) (8, 11);
        };
      ] );
  ]

let class_type =
  "<?hh
class K {}

function test_class(
  class<K> $c,
//   ^5:6
  classname<K> $cn,
//   ^7:6
): void {}"

let class_type_cases =
  [
    ( ("class_type.php", 5, 6),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "class pointer")];
          addendum =
            [
              Lsp.MarkedString
                "For any class C, C::class creates a class<C>. This value is used for
expressions like `new $c()` and `$c::foo()`.";
            ];
          pos = pos_at (5, 3) (5, 7);
        };
      ] );
    ( ("class_type.php", 7, 6),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "classname")];
          addendum =
            [
              Lsp.MarkedString
                "For any class C, nameof C creates a classname<C> string. The typechecker
enforces that C is a defined class, but it does not enforce module or
package boundaries.";
            ];
          pos = pos_at (7, 3) (7, 11);
        };
      ] );
  ]

let docblock =
  "<?hh // strict

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
//          ^17:12
  DocBlockOnClassButNotConstructor::nonConstructorMethod();
//          ^19:37
}

function docblockReturn(): DocBlockBase {
//          ^23:28
  $x = new DocBlockBase();
//          ^25:12
  return new DocBlockDerived();
//          ^27:14
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

  /* Docblock for non-constructor method in DocBlockOnClassButNotConstructor */
  public static function nonConstructorMethod(): void {}
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
function line_comment_with_break(): void { line_comment_with_break(); }
//                                       89^:44

// This is another special case.
/** Only this should be part of the docblock. */
function two_comment_types(): void { two_comment_types(); }
//                                   ^94:38

// There are too many blank lines between this comment and what it's commenting
// on.


function too_many_blank_lines(): void { too_many_blank_lines(); }
//                                      ^101:41

/** A function with an HH_FIXME. */
/* HH_FIXME[4030] Missing return type hint. */
function needs_fixing() { needs_fixing(); }
//                        ^106:27
"

let docblock_cases =
  [
    ( ("docblock.php", 7, 3),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "class DocBlock")];
          addendum =
            [
              Lsp.MarkedString
                "Class doc block.
This
doc
block
has
multiple
lines.";
            ];
          pos = pos_at (7, 3) (7, 10);
        };
      ] );
    ( ("docblock.php", 7, 13),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DocBlock`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public static function doStuff(): void");
            ];
          addendum = [Lsp.MarkedString "Method doc block with double star."];
          pos = pos_at (7, 13) (7, 19);
        };
      ] );
    ( ("docblock.php", 9, 3),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "function queryDocBlocks(): void")];
          addendum = [Lsp.MarkedString "Multiline
function
doc block."];
          pos = pos_at (9, 3) (9, 16);
        };
      ] );
    ( ("docblock.php", 11, 13),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DocBlock`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ("hack", "public static function preserveIndentation(): void");
            ];
          addendum =
            [
              Lsp.MarkedString
                "Multiline doc block with
a certain amount of
    indentation
we want to preserve.";
            ];
          pos = pos_at (11, 13) (11, 31);
        };
      ] );
    ( ("docblock.php", 13, 13),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DocBlock`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ("hack", "public static function leadingStarsAndMDList(): void");
            ];
          addendum =
            [
              Lsp.MarkedString
                "Multiline doc block with
leading stars, as well as
  * a Markdown list!
and we'd really like to preserve the Markdown list while getting rid of
the other stars.";
            ];
          pos = pos_at (13, 13) (13, 33);
        };
      ] );
    ( ("docblock.php", 15, 13),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DocBlock`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ("hack", "public static function manyLineBreaks(): void");
            ];
          addendum =
            [
              Lsp.MarkedString
                "This method has many line breaks, which

someone might use if they wanted

to have separate paragraphs

in Markdown.";
            ];
          pos = pos_at (15, 13) (15, 26);
        };
      ] );
    ( ("docblock.php", 17, 12),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DocBlockOnClassButNotConstructor`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function __construct(): void");
            ];
          addendum =
            [
              Lsp.MarkedString
                "Class doc block for a class whose constructor doesn't have a doc block.";
            ];
          pos = pos_at (17, 12) (17, 43);
        };
      ] );
    ( ("docblock.php", 19, 37),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DocBlockOnClassButNotConstructor`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ("hack", "public static function nonConstructorMethod(): void");
            ];
          addendum =
            [
              Lsp.MarkedString
                "Docblock for non-constructor method in DocBlockOnClassButNotConstructor";
            ];
          pos = pos_at (19, 37) (19, 56);
        };
      ] );
    ( ("docblock.php", 23, 28),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "DocBlockBase")];
          addendum = [Lsp.MarkedString "DocBlockBase: class doc block."];
          pos = pos_at (23, 28) (23, 39);
        };
      ] );
    ( ("docblock.php", 25, 12),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DocBlockBase`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function __construct(): void");
            ];
          addendum = [Lsp.MarkedString "DocBlockBase: constructor doc block."];
          pos = pos_at (25, 12) (25, 23);
        };
      ] );
    ( ("docblock.php", 27, 14),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DocBlockBase`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function __construct(): void");
            ];
          addendum = [Lsp.MarkedString "DocBlockBase: constructor doc block."];
          pos = pos_at (27, 14) (27, 28);
        };
      ] );
    ( ("docblock.php", 89, 44),
      [
        {
          snippet =
            [
              Lsp.MarkedCode ("hack", "function line_comment_with_break(): void");
            ];
          addendum =
            [
              Lsp.MarkedString
                "We don't want the line comment above to be part of this docblock.
We do want both these lines though.";
            ];
          pos = pos_at (89, 44) (89, 66);
        };
      ] );
    ( ("docblock.php", 94, 38),
      [
        {
          snippet =
            [Lsp.MarkedCode ("hack", "function two_comment_types(): void")];
          addendum =
            [Lsp.MarkedString "Only this should be part of the docblock."];
          pos = pos_at (94, 38) (94, 54);
        };
      ] );
    ( ("docblock.php", 101, 41),
      [
        {
          snippet =
            [Lsp.MarkedCode ("hack", "function too_many_blank_lines(): void")];
          addendum = [];
          pos = pos_at (101, 41) (101, 60);
        };
      ] );
    ( ("docblock.php", 106, 27),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "function needs_fixing(): _")];
          addendum = [Lsp.MarkedString "A function with an HH_FIXME."];
          pos = pos_at (106, 27) (106, 38);
        };
      ] );
  ]

let special_cases =
  "<?hh // strict
function special_cases(): void {
  idx(vec[1, 2, 3], 1);
//^3:3
}
"

let special_cases_cases =
  [
    ( ("special_cases.php", 3, 3),
      [
        {
          snippet =
            [
              Lsp.MarkedCode
                ( "hack",
                  "function idx<Tk as arraykey, Tv>(
  ?KeyedContainer<Tk, Tv> $collection,
  ?Tk $index,
  optional HH\\FIXME\\MISSING_PARAM_TYPE $default
)[]: Tv"
                );
              Lsp.MarkedString "---";
              Lsp.MarkedString "Instantiation:";
              Lsp.MarkedCode ("hack", "  Tk = int;
  Tv = ?int;");
            ];
          addendum =
            [
              Lsp.MarkedString
                "Index into the given KeyedContainer using the provided key.

If the key doesn't exist, the key is `null`, or the collection is `null`,
return the provided default value instead, or `null` if no default value was
provided. If the key is `null`, the default value will be returned even if
`null` is a valid key in the container.";
            ];
          pos = pos_at (3, 3) (3, 5);
        };
      ] );
  ]

let bounded_generic_fun =
  "<?hh // strict
abstract class Base {}
final class C extends Base {}
function bounded_generic_fun<T as Base>(T $x): void {
  $x;
//^5:3
  if ($x is C) {
//    ^7:7
    $x;
//  ^9:5
  }
  $x;
//^12:3
}
"

let bounded_generic_fun_cases =
  [
    ( ("bounded_generic_fun.php", 5, 3),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "T as Base")];
          addendum = [];
          pos = pos_at (5, 3) (5, 4);
        };
      ] );
    ( ("bounded_generic_fun.php", 7, 7),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "T as Base")];
          addendum = [];
          pos = pos_at (7, 7) (7, 8);
        };
      ] );
    ( ("bounded_generic_fun.php", 9, 5),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "(T & C)
where T as Base")];
          addendum = [];
          pos = pos_at (9, 5) (9, 6);
        };
      ] );
    ( ("bounded_generic_fun.php", 12, 3),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "T as Base")];
          addendum = [];
          pos = pos_at (12, 3) (12, 4);
        };
      ] );
  ]

let doc_block_fallback =
  "<?hh // strict
function dbfb_func(DBFBClass2 $c, DBFBClass3 $d): void {
  $c->doTheThing();
//    ^3:7
  $c->docBlockInClass();
//    ^5:7
  $c->identical();
//    ^7:7
  $c->slightlyDifferent();
//    ^9:7
  $c->noDocBlock();
//    ^11:7
  $d->docBlockInClass2();
//    ^13:7
  $c->traitFunction();
//    ^15:7
  $d->traitFunction2();
//    ^17:7
}

interface DBFBInterface1 {
  /** DBFBInterface1. */
  public function doTheThing(): void;

  /** Identical. */
  public function identical(): void;

  /** Slightly different. */
  public function slightlyDifferent(): void;

  public function noDocBlock(): void;
}

interface DBFBInterface2 {
  /** DBFBInterface2. */
  public function doTheThing(): void;

  /** DBFBInterface2. */
  public function docBlockInClass(): void;

  /** DBFBInterface2. */
  public function docBlockInClass2(): void;

  /** Identical. */
  public function identical(): void;

  /** Slightly different. */
  public function slightlyDifferent(): void;

  public function noDocBlock(): void;
}

interface DBFBInterface3 {
  /** Slightly more different. */
  public function slightlyDifferent(): void;
}

trait DBFBTrait {
  /** DBFBTrait. */
  public function traitFunction(): void {}

  /** DBFBTrait. */
  public function traitFunction2(): void {}
}

class DBFBClass1 implements DBFBInterface1 {
  use DBFBTrait;

  public function doTheThing(): void {}

  /** DBFBClass1. */
  public function docBlockInClass(): void {}

  /** DBFBClass1. */
  public function docBlockInClass2(): void {}

  public function identical(): void {}

  public function slightlyDifferent(): void {}

  public function noDocBlock(): void {}

  /** DBFBClass1. */
  public function traitFunction2(): void {}
}

class DBFBClass2 extends DBFBClass1 implements DBFBInterface2, DBFBInterface3 {
  public function docBlockInClass2(): void {}
}

class DBFBClass3 extends DBFBClass2 {
  public function docBlockInClass2(): void {}

  public function traitFunction2(): void {}
}
"

let doc_block_fallback_cases =
  [
    ( ("doc_block_fallback.php", 3, 7),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DBFBClass1`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function doTheThing(): void");
            ];
          addendum =
            [
              Lsp.MarkedString
                "DBFBInterface2.
(from DBFBInterface2)

---

DBFBInterface1.
(from DBFBInterface1)";
            ];
          pos = pos_at (3, 7) (3, 16);
        };
      ] );
    ( ("doc_block_fallback.php", 5, 7),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DBFBClass1`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function docBlockInClass(): void");
            ];
          addendum = [Lsp.MarkedString "DBFBClass1."];
          pos = pos_at (5, 7) (5, 21);
        };
      ] );
    ( ("doc_block_fallback.php", 7, 7),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DBFBClass1`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function identical(): void");
            ];
          addendum = [Lsp.MarkedString "Identical."];
          pos = pos_at (7, 7) (7, 15);
        };
      ] );
    ( ("doc_block_fallback.php", 9, 7),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DBFBClass1`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ("hack", "public function slightlyDifferent(): void");
            ];
          addendum =
            [
              Lsp.MarkedString
                "Slightly more different.
(from DBFBInterface3)

---

Slightly different.
(from DBFBInterface1, DBFBInterface2)";
            ];
          pos = pos_at (9, 7) (9, 23);
        };
      ] );
    ( ("doc_block_fallback.php", 11, 7),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DBFBClass1`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function noDocBlock(): void");
            ];
          addendum = [];
          pos = pos_at (11, 7) (11, 16);
        };
      ] );
    (* When falling back, if any class/trait ancestors have a doc block don't show
       any doc blocks from interface ancestors. *)
    ( ("doc_block_fallback.php", 13, 7),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DBFBClass3`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function docBlockInClass2(): void");
            ];
          addendum = [Lsp.MarkedString "DBFBClass1."];
          pos = pos_at (13, 7) (13, 22);
        };
      ] );
    ( ("doc_block_fallback.php", 15, 7),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DBFBTrait`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function traitFunction(): void");
            ];
          addendum = [Lsp.MarkedString "DBFBTrait."];
          pos = pos_at (15, 7) (15, 19);
        };
      ] );
    ( ("doc_block_fallback.php", 17, 7),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `DBFBClass3`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public function traitFunction2(): void");
            ];
          addendum = [Lsp.MarkedString "DBFBClass1."];
          pos = pos_at (17, 7) (17, 20);
        };
      ] );
  ]

let class_id_positions =
  "<?hh // strict
function create_class_id_positions(): void {
  $x = new CIPos(CIPos2::MyConstInt);
//               ^3:18   ^3:26
  $x = new CIPos(CIPos2::$myStaticInt);
//               ^5:18   ^5:26
  $x = new CIPos(CIPos2::returnConstInt());
//               ^7:18   ^7:26
}

class CIPos {
  public function __construct(private int $x) {}
}

class CIPos2 {
  const int MyConstInt = 0;
  public static int $myStaticInt = 1;

  public static function returnConstInt(): int {
    return 2;
  }
}
"

let class_id_positions_cases =
  [
    ( ("class_id_positions.php", 3, 18),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "class CIPos2")];
          addendum = [];
          pos = pos_at (3, 18) (3, 23);
        };
      ] );
    ( ("class_id_positions.php", 3, 26),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `CIPos2`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "const int MyConstInt");
            ];
          addendum = [];
          pos = pos_at (3, 26) (3, 35);
        };
        {
          snippet = [Lsp.MarkedCode ("hack", "Parameter: $x")];
          addendum = [];
          pos = pos_at (3, 18) (3, 35);
        };
      ] );
    ( ("class_id_positions.php", 5, 18),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "class CIPos2")];
          addendum = [];
          pos = pos_at (5, 18) (5, 23);
        };
      ] );
    ( ("class_id_positions.php", 5, 26),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `CIPos2`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode ("hack", "public static int $myStaticInt");
            ];
          addendum = [];
          pos = pos_at (5, 26) (5, 37);
        };
        {
          snippet = [Lsp.MarkedCode ("hack", "Parameter: $x")];
          addendum = [];
          pos = pos_at (5, 18) (5, 37);
        };
      ] );
    ( ("class_id_positions.php", 7, 18),
      [
        {
          snippet = [Lsp.MarkedCode ("hack", "class CIPos2")];
          addendum = [];
          pos = pos_at (7, 18) (7, 23);
        };
      ] );
    ( ("class_id_positions.php", 7, 26),
      [
        {
          snippet =
            [
              Lsp.MarkedString "Defined in `CIPos2`";
              Lsp.MarkedString "---";
              Lsp.MarkedCode
                ("hack", "public static function returnConstInt(): int");
            ];
          addendum = [];
          pos = pos_at (7, 26) (7, 39);
        };
      ] );
  ]

let duplicate_results =
  "<?hh // strict
trait DuplicateResultTrait {
  /** Doc block. */
  public function foo(): void {}
}

class DuplicateResultClass1 {
  use DuplicateResultTrait;
}

class DuplicateResultClass2 {
  use DuplicateResultTrait;
}

function test_duplicate_result_class(bool $x): void {
  if ($x) {
    $y = new DuplicateResultClass1();
  } else {
    $y = new DuplicateResultClass2();
  }

  $y->foo();
//    ^22:7
}
"

let hhconfig = "
allowed_fixme_codes_strict = 4030
"

let files =
  [
    ("class_members.php", class_members);
    ("classname_call.php", classname_call);
    ("chained_calls.php", chained_calls);
    ("classname_variable.php", classname_variable);
    ("class_type.php", class_type);
    ("docblock.php", docblock);
    ("special_cases.php", special_cases);
    ("bounded_generic_fun.php", bounded_generic_fun);
    ("doc_block_fallback.php", doc_block_fallback);
    ("class_id_positions.php", class_id_positions);
  ]

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents =
  "
allowed_fixme_codes_strict = 4030
allowed_decl_fixme_codes = 4030
"

let cases =
  class_id_positions_cases
  @ doc_block_fallback_cases
  @ special_cases_cases
  @ docblock_cases
  @ class_members_cases
  @ classname_call_cases
  @ chained_calls_cases
  @ classname_variable_cases
  @ class_type_cases
  @ bounded_generic_fun_cases

let test () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let (custom_config, _) =
    ServerConfig.load ~silent:false ~from:"" ~cli_config_overrides:[]
  in
  let env =
    Test.setup_server
      ()
      ~custom_config
      ~hhi_files:(Hhi.get_raw_hhi_contents () |> Array.to_list)
  in
  let env = Test.setup_disk env files in
  Test.assert_no_diagnostics env;

  let failed_cases =
    List.filter_map cases ~f:(fun ((file, line, column), expectedHover) ->
        let list_to_string hover_list =
          let string_list =
            hover_list |> List.map ~f:HoverService.string_of_result
          in
          let inner =
            match string_list |> List.reduce ~f:(fun a b -> a ^ "; " ^ b) with
            | None -> ""
            | Some s -> s
          in
          Printf.sprintf "%s:%d:%d: [%s]" file line column inner
        in
        let path = Relative_path.from_root ~suffix:file in
        let (ctx, entry) =
          Provider_context.add_entry_if_missing
            ~ctx:(Provider_utils.ctx_from_server_env env)
            ~path
        in
        let hover =
          Provider_utils.respect_but_quarantine_unsaved_changes
            ~ctx
            ~f:(fun () ->
              Ide_hover.go_quarantined
                ~ctx
                ~entry
                (File_content.Position.from_one_based line column))
        in
        let expected = list_to_string expectedHover in
        let actual = list_to_string hover in
        if not (String.equal expected actual) then
          Some (expected, actual)
        else
          None)
  in
  match failed_cases with
  | [] -> ()
  | cases ->
    let display_case (expected, actual) =
      Printf.sprintf "Expected:\n%s\nGot:\n%s" expected actual
    in
    Test.fail @@ String.concat ~sep:"\n\n" (List.map ~f:display_case cases)
