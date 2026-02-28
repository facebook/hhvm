////file1.php
<?hh
newtype MyType as arraykey = string;

////file2.php
<?hh
function test_bound<T super MyType>(MyType $x): T {
  return $x;
}
