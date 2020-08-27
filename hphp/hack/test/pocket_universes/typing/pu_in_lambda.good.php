<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>


function call<T>((function(): T) $f): T {
  return $f();
}

abstract class A {
  enum E {
    case type T as arraykey;
    case T value;
  }

  abstract public function f<TP as this:@E>(TP $p): TP:@T;

  public function test<TP as this:@E>(TP $p): arraykey {
    $x = call(() ==> $this->f($p));
    return $x;
  }

  public function test2(this:@E $p): arraykey {
    $x = call(() ==> $this->f($p));
    return $x;
  }
}

function test(A $a, A:@E $p): arraykey {
  $x = call(() ==> $a->f($p));
  return $x;
}

function test2<TP as A:@E>(A $a, TP $p): arraykey {
  $x = call(() ==> $a->f($p));
  return $x;
}
