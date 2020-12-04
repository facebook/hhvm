<?hh
<<file:__EnableUnstableFeatures('ifc')>>
class Policy {}

class C {
  <<__Policied("Test")>>
  public function f(): void {}

  <<__Policied("Public")>>
  public function g(): void {}

  <<__Policied>>
  public function implicit(): void {}

  <<__InferFlows>>
  public function inferflows(): void {}

  <<__Policied(Policy::class)>>
  public function classname(): void {}

  // should be default
  public function defaults(): void {}

  public function with_args(
    <<__External>> C $x,
    <<__CanCall>> (function(): void) $f
  ): void {}
}

<<__InferFlows>>
function inferflows(): void {}
