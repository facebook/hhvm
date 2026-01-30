<?hh
// Test go-to-def for instance methods accessed via `require class` constraint

final class C {
  public function instance_meth(): void {}
}

trait T {
  require class C;

  public function test(): void {
    $this->instance_meth();
  }
}
