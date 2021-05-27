<?hh

function example(bool $b): int {
  if ($b) {
    $val = "foo";
  } else {
    $val = 0;
  }

  // $val is (string | int)

  if ($val is string) {
    return 0;
  } else {
    return $val; // $val is int
  }
}
