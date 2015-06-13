<?hh // strict

function f(): void {
  // Unsetting array indexes should be fine
  $a = array('hi');
  unset($a[0]);
}

function g(bool $cond): void {
  if ($cond) {
    $arr = array('foo');
  } else {
    $arr = array(0);
  }
  unset($arr[0]);
}
