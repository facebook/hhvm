////file1.php
<?hh // strict
newtype MyType as arraykey = string;

////file2.php
<?hh // strict
function test_bound<T super MyType>(MyType $x): T {
  return $x;
}
