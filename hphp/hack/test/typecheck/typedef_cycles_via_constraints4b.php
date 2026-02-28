//// defs.php
<?hh

newtype A<T as B<T>> as T = T;
newtype B<T as A<T>> as T = T;

//// uses.php
<?hh

function expect_string(string $x): void {}

function test(A<int> $a, B<int> $b): void {
  expect_string($a);
  expect_string($b);
}
