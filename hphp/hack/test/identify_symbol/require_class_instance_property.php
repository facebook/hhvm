<?hh
// Test go-to-def for instance properties accessed via `require class` constraint

final class C {
  public int $instance_prop = 0;
}

trait T {
  require class C;

  public function test(): void {
    $_ = $this->instance_prop;
  }
}
