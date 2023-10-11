<?hh

class DerivedClassIncremental extends BaseClassIncremental {
  public function bar(): string { return ''; }
}

function uses_base(DerivedClassIncremental $derived): void {
  $derived->foo();
}
