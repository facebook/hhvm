<?hh

module MLT_A;

// Public traits cannot define internal methods

trait T {
  public function f1(): void {}
  private function f2(): void {}
  protected function f3(): void {}
  internal function f4(): void {}
}
