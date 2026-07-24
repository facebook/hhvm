//// isolated/c.php
<?hh
// A class in the strict-isolation package `isolated`.
class IsolatedType2 {}

//// isolated/use.php
<?hh
// Same package (`isolated`) may reference its own class in type positions.
function takes_isolated(IsolatedType2 $x): vec<IsolatedType2> {
  return vec[$x];
}
