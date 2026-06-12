<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

// `require this as C` and `require class C` for the same target are compatible
// (the subtype constraint is implied by the exact-class constraint). The
// stricter `require class` must be propagated, not reported as a conflict.

trait ReqThisAsTrait {
  require this as C;
}

trait ReqClassTrait {
  require class C;
}

final class C {
  use ReqThisAsTrait;
  use ReqClassTrait;

  public function hello(): void { echo "hello\n"; }
}


<<__EntryPoint>>
function main() : void {
  (new C())->hello();
}
