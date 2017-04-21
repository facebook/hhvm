<?hh

function f(mixed $k): keyset<string> {
  if (is_keyset($k)) {
    return $k;
  } else {
    return keyset[];
  }
}
