<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definitions so we don't get naming errors.
class Code {}
class Foo {
  public static function bar(): int { return 1; }
}

const int MY_CONST = 1;

function foo(): void {
  // Ban binary operators.
  $x = Code`1 << 2`;

  // Ban static methods and instantiation.
  $y = Code`Foo::bar()`;
  $z = Code`new Foo()`;

  // Ban globals.
  $g = Code`MY_CONST + 1`;

  // Ban PHP-style lambdas.
  $f = Code`function() { return 1; }`;

  // Ban loops other than while.
  $f = Code`() ==> { do {} while(true); }`;
  $f = Code`() ==> { for($i = 0; $i < 10; $i++) {} }`;

  // Ban lambdas with default arguments.
  $f = Code`($x = 1) ==> { return $x; }`;

  // Ban assignment to things that aren't simple variables.
  $f = Code`(dynamic $x) ==> { $x[0] = 1; }`;
  $f = Code`(dynamic $x) ==> { $x->foo = 1; }`;

  // Ban assignments that mutate a local.
  $f = Code`($x) ==> { $x += 1; }`;
}
