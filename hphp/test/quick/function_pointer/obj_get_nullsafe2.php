<?hh

final class MyClass {
  public function foo(int $_): void {
    echo "From foo with regards\n";
  }
}

function call_foo(?MyClass $x): void {
  $y = $x?->foo<>;

  if ($y is null) {
    echo "MyClass was null\n";
  } else {
    $y("Wrong input");
  }
}

<<__EntryPoint>>
function test(): void {
  $z = new MyClass();

  call_foo($z);
}
