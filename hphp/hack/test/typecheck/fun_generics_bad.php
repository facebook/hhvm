<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function cannot_be_nullable(int $val): int {
  return $val;
}

/* HH_FIXME[4110] */
function special_array_map<T1, T2>(
  (function(T1): T2) $f,
  darray<int, T1> $a,
): darray<int, T2> {
}

<<__EntryPoint>>
function demo(): bool {
  $array = dict[];

  for ($k = 0; $k < 10; $k++) {
    $array[$k] = null;
  }

  $fun1 = cannot_be_nullable<>;

  $val1 = special_array_map($fun1, $array);

  return (bool)$val1;
}
