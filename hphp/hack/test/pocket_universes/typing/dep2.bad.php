<?hh //strict

class C {
  enum E {
    case type T;
    case T value;
    :@I(type T = int, value = 42);
  }

  public static function f<TP as this:@E>(TP $atom, this:@E:@TP:@T $x) :void {
  }
}

function expect<T>(T $_): void {}
function id<T>(T $x) : T { return $x; }

function testit() : void {
  C::f(:@I, 42);
  C::f(:@I, "42");
}
