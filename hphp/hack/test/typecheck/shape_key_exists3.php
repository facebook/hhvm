<?hh // strict

// Nonexisting field in ad-hoc shape

function test(): void {
  $s = shape();
  Shapes::idx($s, 'y');
}
