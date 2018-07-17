<?hh // strict

// __PPL attributed classes have special rewriting rules for
// sample, factor, observe, and condition
// They are typed as if they were calls made to Infer->sample, etc.

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
  public function condition(int $s): int {
    return 0;
  }
}

<<__PPL>>
class MyClass {
  public function test(): void {
    $x = sample(1);
    $x = factor($x);
    $x = observe($x);
    condition($x);
  }
}
