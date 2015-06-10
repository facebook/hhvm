<?hh // strict

function f(): void {
  // Unsetting array indexes should be fine
  $a = array('hi');
  unset($a[0]);
}
