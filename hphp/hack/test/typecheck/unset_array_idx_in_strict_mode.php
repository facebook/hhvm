<?hh

function f(): void {
  // Unsetting array indexes should be fine
  $a = vec['hi'];
  unset($a[0]);
}

function g(bool $cond): void {
  if ($cond) {
    $arr = vec['foo'];
  } else {
    $arr = vec[0];
  }
  unset($arr[0]);
}
