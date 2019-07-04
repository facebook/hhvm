<?hh // strict

// __PPL attributed classes have special rewriting rules for
// sample, factor, observe, condition, and sample_model
// They are typed as if they were calls made to Infer->sample, etc.
// Infer is a special class defined by the PPL team

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
  public function sample_model(int $s): int {
    return 0;
  }
}

<<__PPL>>
class MyClass {
  public function test(): void {
    sample_model("hi");
  }
}
