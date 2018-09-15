<?hh // strict

function sample(string $k): void {}
function observe(string $k): void {}
function condition(string $k): void {}
function factor(string $k): void {}
function sample_model(string $k): void {}

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
  public function sample_model(int $k): int {
    return 0;
  }
}

<<__PPL>>
class MyClass {
  public function foo(): void {}

  public function my_lambda(): void {
    $x = function(): void {
      factor("hi");
    };
  }
}
