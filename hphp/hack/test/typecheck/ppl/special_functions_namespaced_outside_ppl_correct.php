//// Infer.php
<?hh // strict

// There are five special functions that are rewritten in <<__PPL>> annotated
// files. They should be typechecked as if they were calls to `Infer->method`
// in those cases, but we want to make sure that when they are not in a
// <<__PPL>> file, they refer to the correct functions, in the namespace and
// globally.

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

function sample(vec<string> $k): void {}
function observe(vec<string> $k): void {}
function condition(vec<string> $k): void {}
function factor(vec<string> $k): void {}
function sample_model(vec<string> $k): void {}

//// PPL.php
<?hh // strict

namespace PPL\Test;

function sample(string $k): void {}
function observe(string $k): void {}
function condition(string $k): void {}
function factor(string $k): void {}
function sample_model(string $k): void {}

class MyClass {
  public function test(): void {
    sample("hi");
    factor("hi");
    observe("hi");
    condition("hi");
    sample_model("hi");

    \sample(vec[]);
    \factor(vec[]);
    \observe(vec[]);
    \condition(vec[]);
    \sample_model(vec[]);
  }
}
