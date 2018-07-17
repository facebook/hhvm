//// Infer.php
<?hh // strict

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

function sample(vec<string> $k): void {}
function observe(vec<string> $k): void {}
function condition(vec<string> $k): void {}
function factor(vec<string> $k): void {}

//// PPL.php
<?hh // strict

namespace PPL\Test;

function sample(string $k): void {}
function observe(string $k): void {}
function condition(string $k): void {}
function factor(string $k): void {}

class MyClass {
  public function test(): void {
    sample("hi");
    factor("hi");
    observe("hi");
    condition("hi");

    \sample(vec[]);
    \factor(vec[]);
    \observe(vec[]);
    \condition(vec[]);
  }
}
