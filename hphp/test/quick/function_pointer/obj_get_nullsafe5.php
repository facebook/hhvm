<?hh

final class MyClass {
  public function bar<T>(T $x): void {
    echo "Inside bar\n";
  }
}

function call_bar(?MyClass $x): void {
  $y = $x?->bar<string>;
  // Hack error, but HHVM should be fine with this
  $y(4);
}

<<__EntryPoint>>
function test(): void {
  $z = new MyClass();

  call_bar($z);
}
