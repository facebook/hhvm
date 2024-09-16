//// defs.php
<?hh

// Transparent type aliases like `type X = int`cannot have constraints

newtype A as B = int;

newtype B as A = int;

//// uses.php
<?hh

function expect_string(string $x): void {}

function test(A $a, B $b): void {
  expect_string($a);
  expect_string($b);
}
