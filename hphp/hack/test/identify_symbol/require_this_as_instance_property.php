<?hh
// Test go-to-def for instance properties accessed via `require this as` constraint

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
  public int $instance_prop = 0;
}

trait T {
  require this as C;

  public function test(): void {
    $_ = $this->instance_prop;
  }
}
