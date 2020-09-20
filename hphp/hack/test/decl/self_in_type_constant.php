<?hh

class C {
  const type T = int;
  const type U = self::T;
}

interface I {
  const type V = int;
  const type W = self::V;
}

trait T {
  require extends C;
  abstract public function f(): self::U;
}
