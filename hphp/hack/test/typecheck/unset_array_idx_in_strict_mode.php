<?hh

function f(): void {
  // Unsetting array indexes should be fine
  $a = varray['hi'];
  unset($a[0]);
}

function g(bool $cond): void {
  if ($cond) {
    $arr = varray['foo'];
  } else {
    $arr = varray[0];
  }
  unset($arr[0]);
}
