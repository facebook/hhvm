<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class Target {
  public function __construct(named readonly int $r, named int $m) {}

  public function method(named readonly int $r, named int $m): void {}
}

function target(named readonly int $r, named int $m): void {}

function test(readonly int $ro): void {
  new Target(r = $ro, m = 2);
  new Target(m = 2, r = $ro);
  target(r = 1, m = 2);
  target(m = 2, r = 1);
  target(r = $ro, m = 2);
  target(m = 2, r = $ro);
  target(m = 2, r = readonly 1);

  $f = target<>;
  $f(m = 2, r = $ro);
  $target = new Target(m = 2, r = $ro);
  $target->method(m = 2, r = $ro);
}
//
