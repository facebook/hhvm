<?hh
// Test go-to-def for instance methods accessed via `require this as` constraint

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
  public function instance_meth(): void {}
}

trait T {
  require this as C;

  public function test(): void {
    $this->instance_meth();
  }
}
