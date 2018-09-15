//// Infer.php
<?hh // strict

class Infer {
  public function sample(int $k): int {
    return 0;
  }
}

function sample(string $k): void {}

//// PPL.php
<?hh // strict

namespace PPL\Test;

function sample(string $k): void {}

class MyClass {
  public function test(): void {
    \sample("hi");
  }
}
