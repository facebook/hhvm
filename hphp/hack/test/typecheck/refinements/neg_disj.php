<?hh

function example(bool $b1, bool $b2): int {
  if ($b1) {
    $val = "foo";
  } else if ($b2) {
    $val = 0;
  } else {
    $val = true;
  }

  // $val is (string | int | bool)

  if ($val is string || $val is bool) {
    return 0;
  } else {
    return $val;  // $val is int
  }
}
