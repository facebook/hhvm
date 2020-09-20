<?hh // partial

class A {
  protected int $val = 43;

  public function b(): int {
    return $this->val - 1;
  }
}

echo (new A())->b(), "\n";
