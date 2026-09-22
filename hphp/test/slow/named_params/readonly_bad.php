<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function named_readonly(int $m, named readonly int $r): void {}

function reversed_names(int $m, named readonly int $a, named int $z): void {}

class C {
  public function __construct(int $m, named readonly int $r) {}
}

<<__EntryPoint>>
function main(): void {
  try {
    named_readonly(readonly 1, r=2);
  } catch (ReadonlyViolationException $e) {
    echo "function rejected\n";
  }
  try {
    reversed_names(readonly 1, z=3, a=2);
  } catch (ReadonlyViolationException $e) {
    echo "reversed names rejected\n";
  }
  try {
    new C(readonly 1, r=2);
  } catch (ReadonlyViolationException $e) {
    echo "constructor rejected\n";
  }
}
