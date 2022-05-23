//// file1.php
<?hh

newtype N<+T> = int;

//// file2.php
<?hh

// newtypes are not necessarily injective (as illustrated by N), meaning that
// we have
// T1 <: T2 implies N<T1> <: N<T2>
// but the reverse direction does not hold in general.
function badcast<T1,T2>(T1 $x):T2 where N<T1> as N<T2> {
  return $x;
}
