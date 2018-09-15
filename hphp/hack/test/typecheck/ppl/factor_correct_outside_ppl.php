<?hh // strict

function sample(string $k): void {}
function observe(string $k): void {}
function condition(string $k): void {}
function factor(string $k): void {}

class Infer {
  public function sample(int $k): int {
    return 0;
  }
  public function factor(int $s): int {
    return 0;
  }
  public function observe(int $k): int {
    return 0;
  }
  public function condition(int $s): int {
    return 0;
  }
}

class MyClass {
  public function my_method(): void {
    factor("hi");
  }
}
