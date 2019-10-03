<?hh

function unset_is_ok(
  dynamic $dyn,
  dict<int, string> $dict,
  ~dict<int, bool> $like,
): void {
  unset($dyn[0]);
  unset($dict[0]);
  unset($like[0]);

  if ($dyn) {
    $x = $dyn;
  } else {
    $x = $dict;
  }

  unset($x[0]);
}
