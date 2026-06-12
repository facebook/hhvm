<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

// When `require this as C` and `require class C` collide on the same target,
// the stricter `require class` must be propagated. Here D inherits
// `require this as C` from its parent C (which D satisfies, being a subtype of
// C) but uses a trait declaring `require class C`. The merged-in `require
// class` is the stricter constraint and must still be enforced, so using the
// trait from D (which is not exactly C) is a fatal error -- it must NOT be
// silently accepted as if only `require this as C` applied.

trait ReqThisAsTrait {
  require this as C;
}

class C {
  use ReqThisAsTrait;
}

trait ReqClassTrait {
  require class C;
}

final class D extends C {
  use ReqClassTrait;
}


<<__EntryPoint>>
function main() : void {
  new D();
  echo "done\n";
}
