<?hh

final class MyClass {
  public ?MyClass $foo = null;

  public function bar(int $x): void {
  }
}

function call_bar(?MyClass $x): void {
  $y = $x?->foo?->bar<>;

  if ($y is null) {
    echo "Was null\n";
  } else {
    $y();
  }
}

<<__EntryPoint>>
function test(): void {
  $z = new MyClass();

  call_bar($z);

  $z2 = null;

  call_bar($z);
}
