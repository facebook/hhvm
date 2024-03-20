<?hh

function f(mixed $value): bool {
  if ($value is bool) {
    return true;
  } else if ($value is dict<_, _>) {
    foreach ($value as $key => $value) {
      return f($value);
    }
    return true;
  }

  return false;
}
