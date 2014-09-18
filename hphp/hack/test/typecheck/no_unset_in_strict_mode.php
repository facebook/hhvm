<?hh // strict

function f(): void {
  // Yes, this is technically okay, but we've deliberately decided to ban it.
  // cf. https://github.com/facebook/hhvm/issues/2711#issuecomment-43657444
  $a = array('hi');
  unset($a[0]);
}
