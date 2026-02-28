//// defs.php
<?hh

newtype A<T as A<T>> as T = T;


//// uses.php
<?hh

function expect_string(string $x): void {}

function test(A<int> $a): void {
  expect_string($a);
}
