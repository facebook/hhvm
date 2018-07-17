//// Infer.php
<?hh // strict

class Infer {
  public function sample(string $k): string {
    return "";
  }
  public function factor(string $s): int {
    return 0;
  }
  public function observe(string $k): string {
    return "";
  }
  public function condition(string $s): int {
    return 0;
  }
}

function sample(int $k): void {}
function observe(int $k): void {}
function condition(int $k): void {}
function factor(int $k): void {}

//// PPL.php
<?hh // strict

namespace PPL\Test;

// __PPL attributed classes have special rewriting rules for
// sample, factor, observe, and condition
// They are typed as if they were calls made to Infer->sample, etc.
// Infer is a special class defined by the PPL team
// This file should produce no errors

function sample(int $k): void {}
function observe(int $k): void {}
function condition(int $k): void {}
function factor(int $k): void {}

<<__PPL>>
class MyClass {
  public function test(): void {
    $x = sample("hi");
    $x = factor("hi");
    $x = observe("hi");
    condition("hi");

    $x = \sample("hi");
    $x = \factor("hi");
    $x = \observe("hi");
    \condition("hi");
  }
}
