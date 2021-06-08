<?hh


interface I {}
class Box implements I {
  public function __construct(public int $i)[] {}
}
class Crate implements I {
  public function __construct(public string $s)[] {}
}

enum class EE : I {
  Box MemBox = new Box(42);
  Crate MemCrate = new Crate("hello");
}

function get<T>(<<__ViaLabel>> HH\MemberOf<EE, T> $z) : T {
  return $z;
}

function test(): void {
  get#AUTO332
}
