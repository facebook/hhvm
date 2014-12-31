<?hh // strict

function g(): void {}

function f(): void {
  // Yes, this is technically okay at runtime, but we've deliberately
  // decided to ban it due to its error suppression properties.
  $a = array('hi');
  if (!empty($a[0])) {
    g();
  }
}
