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
  public function sample_model(string $s): int {
    return 0;
  }
  public function sampleiid(string $s): string {
    return "";
  }
  public function observeiid(string $s): string {
    return "";
  }
}

function sample(int $k): void {}
function observe(int $k): void {}
function condition(int $k): void {}
function factor(int $k): void {}
function sample_model(int $k): void {}
function sampleiid(int $k): void {}
function observeiid(int $k): void {}

//// PPL.php
<?hh // strict

namespace PPL\Test;

// __PPL attributed classes have special rewriting rules for
// sample, factor, observe, condition, sample_model, sampleiid and observeiid
// They are typed as if they were calls made to Infer->sample, etc.
// Infer is a special class defined by the PPL team
// This file should produce no errors

function sample(int $k): void {}
function observe(int $k): void {}
function condition(int $k): void {}
function factor(int $k): void {}
function sample_model(int $k): void {}
function sampleiid(int $k): void {}
function observeiid(int $k): void {}

<<__PPL>>
class MyClass {
  public function test(): void {
    $x = sample("hi");
    $x = factor("hi");
    $x = observe("hi");
    $x = condition("hi");
    $x = sample_model("hi");
    $x = sampleiid("hi");
    observeiid("hi");

    $x = \sample("hi");
    $x = \factor("hi");
    $x = \observe("hi");
    $x = \condition("hi");
    $x = \sample_model("hi");
    $x = \sampleiid("hi");
    \observeiid("hi");
  }
}
