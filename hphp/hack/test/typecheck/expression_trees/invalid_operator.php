<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public static function bar(): int { return 1; }
}

const int MY_CONST = 1;

function foo(): void {
  // Ban instantiation.
  $z = Code`new Foo()`;

  // Ban globals.
  $g = Code`MY_CONST + 1`;

  // Ban PHP-style lambdas.
  $f = Code`function() { return 1; }`;

  // Ban do-while and foreach loops.
  $f = Code`() ==> { do {} while(true); }`;
  $f = Code`(vec<int> $items) ==> { foreach ($items as $_) {} }`;

  // Ban lambdas with default arguments.
  $f = Code`(ExampleInt $x = 1) ==> { return $x; }`;

  // Ban assignment to things that aren't simple variables.
  $f = Code`(dynamic $x) ==> { $x[0] = 1; }`;
  $f = Code`(dynamic $x) ==> { $x->foo = 1; }`;

  // Ban assignments that mutate a local.
  $f = Code`(int $x) ==> { $x += 1; }`;
}
