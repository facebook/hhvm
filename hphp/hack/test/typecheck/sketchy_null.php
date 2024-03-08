<?hh

function f(mixed $m): null {
  if ($m) {
    return null;
  } else {
    return $m;
  }
}
