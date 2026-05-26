<?hh

class C {
  private ?int $foo = null;

  public function get(): void {
    if ($this->foo is nonnull) {
      $this->foo;
    }
  }
}
