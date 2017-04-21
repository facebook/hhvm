<?hh

function foo(mixed $m): keyset<arraykey> {
  if (is_keyset($m)) {
    return $m;
  } else {
    return keyset[];
  }
}
