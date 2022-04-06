<?hh

function takes_stringish_object(StringishObject $o): void {
}

final class MyClass {
  public function __toString(): string {
    return "MyClass";
  }
}

function test(): void {
  $x = new MyClass();

  takes_stringish_object($x);
}
