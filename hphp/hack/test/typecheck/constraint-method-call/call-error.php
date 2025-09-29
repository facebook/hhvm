<?hh

class C {
  public function f(int $t, string $u): void {}

  public function g(): void {
    $this->f(1, 2);
  }

  public function h(): void {
    $this->missing();
  }
}
