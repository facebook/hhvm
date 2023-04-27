<?hh

class C {
  public function splat_m(num ...$n): void {}
}

function main1(C $c): void { // TODO: too conservative, we don't need to invalidate splatted dicts
  $c->splat_m(...dict["a" => 42, "b" => 42.0]);
}

function splat_f(num ...$n): void {}

function main2(): void {
  splat_f(...dict["a" => 42, "b" => 42.0]);
}
