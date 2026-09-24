<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class Target {
  public function __construct(named readonly int $r, named int $m) {}

  public function method(named readonly int $r, named int $m): void {}
}

function target(named readonly int $r, named int $m): void {}

function test(readonly int $ro): void {
  new Target(r = 2, m = $ro);
  new Target(m = $ro, r = 2);

  target(r = 2, m = $ro);
  target(m = $ro, r = 2);
  target(m = readonly 1, r = 2);

  $f = target<>;
  $f(m = $ro, r = 2);
  $target = new Target(m = 2, r = $ro);
  $target->method(m = $ro, r = 2);
}
//
