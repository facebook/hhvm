<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class NamedDup {
  public function f(named int $n): void {}
}

function test_constraint_dup_ok(): void {
  $c = new NamedDup();
  $c->f(n = 1);
}

function test_constraint_dup_err(): void {
  $c = new NamedDup();
  // Error: duplicate named argument 'n'
  $c->f(n = 1, n = "wrong");
}
