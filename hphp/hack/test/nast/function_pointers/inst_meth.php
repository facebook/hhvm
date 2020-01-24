<?hh

namespace Foo\Bar;

final class Qux {
  public function buzz(): void {}
}

function baz(Qux $qux): void {
  // No elaboration needed for inst_meth like items
  $qux->buzz<>;
}
