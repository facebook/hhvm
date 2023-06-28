<?hh

class C {
  const X = 1;

  static function f() :mixed{
    return new C();
  }

  const Y = C::f()::X;
}

