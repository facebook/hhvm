<?hh // strict

function ts<T>(typename<T> $tn): TypeStructure<T> {
  // UNSAFE
}

function swap<T1, T2>(
  TypeStructure<(T1, T2)> $ts,
): (TypeStructure<T2>, TypeStructure<T1>) {
  list($t1, $t2) = $ts['elem_types'];
  return tuple($t2, $t1);
}

function test(
  typename<int> $t1,
  typename<shape(
    'x' => ?int,
    'y' => (string, bool),
  )> $t2,
): void {
  $ts1 = ts($t1);
  $ts2 = ts($t2);
  hh_show($ts1);
  hh_show($ts2);
  hh_show(swap($ts2['fields']['y']));
}
