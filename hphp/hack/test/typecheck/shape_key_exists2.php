<?hh // strict

// Nonexisting field in declared shape

type s = shape('x' => int);

function test(s $s): void {
  Shapes::idx($s, 'y');
}
