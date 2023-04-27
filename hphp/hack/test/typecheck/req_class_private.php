<?hh
<<file:__EnableUnstableFeatures("require_class")>>

final class DemoClass {
  use TDemo;
  private function forPrivate(): void {}
}

trait TDemo {
  require class DemoClass;

  public function demoFunc(): void {
    $this->forPrivate();
  }
}
