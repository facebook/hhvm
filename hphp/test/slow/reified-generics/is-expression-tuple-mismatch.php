<?hh

// Test case for T252896565: segfault when checking reified generic is tuple
// when the actual type is not a tuple

final class Box<reify T> {}

<<__EntryPoint>>
function main(): void {
  $with_string = new Box<string>();
  $with_int = new Box<int>();
  $with_tuple = new Box<(string, int)>();

  echo "Testing tuple on tuple (should work):\n";
  var_dump($with_tuple is Box<(string, int)>);  // true
  var_dump($with_tuple is Box<(int, string)>);  // false

  echo "\nTesting non-tuple types (should work):\n";
  var_dump($with_string is Box<string>);  // true
  var_dump($with_int is Box<int>);        // true

  echo "\nTesting tuple on non-tuple (should return false, not segfault):\n";
  var_dump($with_string is Box<(string)>);  // false

  echo "\nAll tests passed!\n";
}
