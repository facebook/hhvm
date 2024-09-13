//// defines.php
<?hh
newtype B = int;
newtype A as A = int;

//// uses.php
<?hh

function expect_string(string $x): void {}

function test(A $a): void {
  expect_string($a);
}
