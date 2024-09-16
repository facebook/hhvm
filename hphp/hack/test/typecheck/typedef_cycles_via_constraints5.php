//// defs.php
<?hh

newtype A<T> as B<A<T>> = T;
newtype B<T> as C<B<T>> = T;
newtype C<T> as T = T;


//// uses.php
<?hh

function expect_string(string $x): void {}

function test(A<int> $a): void {
  expect_string($a);
}
