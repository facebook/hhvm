<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

enum class E : int {
  int A = 42;
}

interface I {
  public function f(<<__ViaLabel>>HH\MemberOf<E, int> $x): int;
}

class C implements I {
  public function f(HH\MemberOf<E, int> $x): int {
    return $x;
  }
}

class D {
  public function f(<<__ViaLabel>>HH\MemberOf<E, int> $x): int {
    return $x;
  }
}

class F extends D {
  <<__Override>>
  public function f(HH\MemberOf<E, int> $x): int {
    return $x;
  }
}

trait T {
  public function f(HH\MemberOf<E, int> $x): int {
    return $x;
  }
}

class X implements I {
  use T;
}

class Y extends D {
  use T;
}

trait T0 {
  require implements I;

  public function f(HH\MemberOf<E, int> $x): int {
    return $x;
  }
}

trait T1 {
  require extends D;

  public function f(HH\MemberOf<E, int> $x): int {
    return $x;
  }
}
