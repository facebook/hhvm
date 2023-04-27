<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public static function bar(): int { return 1; }
}

const int MY_CONST = 1;

function foo(): void {
  // Ban instantiation.
  $z = ExampleDsl`new Foo()`;

  // Ban globals.
  $g = ExampleDsl`MY_CONST + 1`;

  // Ban PHP-style lambdas.
  $f = ExampleDsl`function() { return 1; }`;

  // Ban do-while and foreach loops.
  $f = ExampleDsl`() ==> { do {} while(true); }`;
  $f = ExampleDsl`(vec<int> $items) ==> { foreach ($items as $_) {} }`;

  // Ban lambdas with default arguments.
  $f = ExampleDsl`(ExampleInt $x = 1) ==> { return $x; }`;

  // Ban assignment to things that aren't simple variables.
  $f = ExampleDsl`(dynamic $x) ==> { $x[0] = 1; }`;

  // Ban assignments that mutate a local.
  $f = ExampleDsl`(int $x) ==> { $x += 1; }`;
}
