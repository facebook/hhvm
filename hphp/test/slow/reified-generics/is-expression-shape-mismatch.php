<?hh

// Test case for T252896565: segfault when checking reified generic is shape
// when the actual type is not a shape

final class Foo<reify T> {}

type MyShape = shape(
  'bar' => int,
);

type OtherShape = shape(
  'barz' => int,
);

class MyTypeClass {}

<<__EntryPoint>>
function main(): void {
  $with_string = new Foo<string>();
  $with_myshape = new Foo<MyShape>();
  $with_mytypeclass = new Foo<MyTypeClass>();

  // These should all work
  echo "Testing string checks:\n";
  var_dump($with_string is Foo<string>);       // true
  var_dump($with_mytypeclass is Foo<string>);  // false
  var_dump($with_myshape is Foo<string>);      // false

  echo "\nTesting class checks:\n";
  var_dump($with_string is Foo<MyTypeClass>);     // false
  var_dump($with_myshape is Foo<MyTypeClass>);    // false
  var_dump($with_mytypeclass is Foo<MyTypeClass>); // true

  echo "\nTesting shape on shape (should work):\n";
  var_dump($with_myshape is Foo<MyShape>);     // true
  var_dump($with_myshape is Foo<OtherShape>);  // false

  echo "\nTesting shape on non-shape (should return false, not segfault):\n";
  var_dump($with_string is Foo<MyShape>);      // false
  var_dump($with_mytypeclass is Foo<MyShape>); // false

  echo "\nAll tests passed!\n";
}
