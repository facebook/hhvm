//// ty.php
<?hh

newtype NewType = mixed;

//// foo.php
<?hh

interface IFoo {}

function foo<T1 as NewType, T2 as T1, T3 as T2, T4 as T3, T5 as T4>(
  T5 $x,
): void {
  if ($x is bool) {
  }
}
