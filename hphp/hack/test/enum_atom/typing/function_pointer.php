<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

enum class E : int {
  int A = 42;
}

class C {
  public static function f(HH\MemberOf<E, int> $_): void {
    echo "YOLO\n";
  }

  public static function g(<<__Atom>>HH\MemberOf<E, int> $_): void {
    echo "YOLO\n";
  }
}

class D extends C {
  public function testit(): void {
    $fp = static::f<>;
    $fp(E::A);

    $fp2 = static::g<>;
  }
}

function h(<<__Atom>>HH\MemberOf<E, int> $_): void {}

function bad(): void {
  $d = new D();
  $d->testit();
  $h = h<>;
}
