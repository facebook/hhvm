<?hh

function map<T1, T2>(T1 $t1, (function(T1): T2) $t2): T2 {
  return $t2($t1);
}
function f(): void {
  $x = map(1, HH\FIXME\UNSAFE_CAST<int, string>);
}
