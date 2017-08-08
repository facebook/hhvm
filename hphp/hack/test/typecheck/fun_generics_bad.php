<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function cannot_be_nullable(int $val): int {
  return $val;
}

function special_array_map<T1, T2>(
  (function(T1): T2) $f,
  array<int, T1> $a,
): array<int, T2> {
  // UNSAFE
}

function demo(): bool {
  $array = array();

  for ($k = 0; $k < 10; $k++) {
    $array[$k] = null;
  }

  $fun1 = fun('cannot_be_nullable');

  $val1 = special_array_map($fun1, $array);

  return (bool) $val1;
}

/* HH_FIXME[1002] Just a demo */
demo();
