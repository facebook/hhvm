<?hh // strict
class E extends Exception {}

function test_invariant(): void {
  invariant_violation('This is dead code');
  new E();
}

function test_if_false(): void {
  if (false) {
    new E();
  }
}

function test_try_catch(): void {
  try {} catch (E $_) { new E(); }
  try { () ==> {}(); } catch (E $_) { new E(); }
}
