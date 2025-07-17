<?hh

function my_map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  return vec[];
}

function myrepro(): dict<string, mixed> {
  $output = my_map(vec[], $_ ==> shape());

  $merged_output = dict[];
  foreach ($output as $_key => $value) {
    if ($value is dict<_, _>) {
      foreach ($value as $k => $v) {
        $merged_output[$k as string] = $v;
      }
    }
  }

  return $merged_output;
}
