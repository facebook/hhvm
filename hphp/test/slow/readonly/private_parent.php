<?hh

class D {
  public static function get(): readonly D {
    return new D();
  }
}

abstract class A {
  public function __construct(private readonly D $d) {}
}

final class B extends A {
  public static function get(readonly D $d): B {
    return new self($d);
  }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(B::get(readonly D::get()));
}
