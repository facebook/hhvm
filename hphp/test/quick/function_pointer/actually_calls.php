<?hh

// Tests function calls with empty type arguments.
// Function calls with empty type arguments was added as part of the first class
// function pointer work. Previously they were not allowed.

function my_test(string $x): void {
  echo $x;
}

function ident<T>(T $x): T {
  echo $x;
  return $x;
}

final class MyTest {
  public function foo(string $x): void {
    echo $x;
  }
}

function test_nullsafe(?MyTest $z): void {
  $z?->foo<>(ident<string>("Should print only once\n"));
}

<<__EntryPoint>>
function main(): void {
  my_test<>("Hello World, my_test\n");

  $y = new MyTest();

  $y->foo<>("Inside foo\n");

  test_nullsafe<>(null);

  echo "Goodbye\n";
}
