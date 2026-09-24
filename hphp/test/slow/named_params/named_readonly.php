<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class Target {
  public function __construct(named readonly int $r, named int $m = 2) {
    echo "$r $m\n";
  }
}

function target(named readonly int $r, named int $m = 2): void {
  echo "$r $m\n";
}

<<__NEVER_INLINE>>
function test(readonly int $ro): void {
  new Target(r = $ro, m = 2);
  new Target(m = 2, r = $ro);
  new Target(r = $ro);
  target(r = $ro, m = 2);
  target(m = 2, r = $ro);
  target(r = $ro);
  target(m = 2, r = readonly 1);
  $f = __hhvm_intrinsics\launder_value(target<>); // make opaque to optimizer
  $f(r = $ro);
}

<<__EntryPoint>>
function main(): void {
  test(readonly 1);
}
