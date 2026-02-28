<?hh

function ts<T>(typename<T> $tn): TypeStructure<T> {
  throw new Exception();
}

function swap<T1, T2>(
  TypeStructure<(T1, T2)> $ts,
): (TypeStructure<T2>, TypeStructure<T1>) {
  list($t1, $t2) = $ts['elem_types'];
  return tuple($t2, $t1);
}
function expectTSint(TypeStructure<int> $ts):void { }

function test(
  typename<int> $t1,
  typename<shape(
    'x' => ?int,
    'y' => (string, bool),
    ...
  )> $t2,
): void {
  $ts1 = ts($t1);
  $ts2 = ts($t2);
  expectTSint($ts1);
  swap($ts2['fields']['y']);
}
