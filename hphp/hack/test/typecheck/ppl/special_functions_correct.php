<?hh // strict

// __PPL attributed classes have special rewriting rules for
// sample, factor, observe, condition and sample_model
// They are typed as if they were calls made to Infer->sample, etc.
// Infer is a special class defined by the PPL team
// This file should produce no errors

function sample(int $k): void {}
function observe(int $k): void {}
function condition(int $k): void {}
function factor(int $k): void {}
function sample_model(int $k): void {}

class Infer {
  public function sample(int $k): string {
    return "";
  }
  public function factor(string $s): int {
    return 0;
  }
  public function observe(int $k): string {
    return "";
  }
  public function condition(string $s): int {
    return 0;
  }
  public function sample_model(int $k): string {
    return "";
  }
}

<<__PPL>>
class MyClass {
  public function test(): void {
    $x = sample(1);
    $x = factor($x);
    $x = observe($x);
    $x = condition($x);
    $x = sample_model($x);
  }
}
