<?hh

function round_trip_in_apc(mixed $val): mixed {
  $err = null;
  apc_store('abc', $val);
  return apc_fetch('abc', inout $err);
}

function foo($v) {
  return varray[$v];
}

<<__EntryPoint>>
function main() {
  $a = foo(10);
  $a = HH\array_mark_legacy($a);
  unset($a[0]);
  $b = round_trip_in_apc(dict['a' => $a]);
  var_dump($b);
  var_dump(HH\is_array_marked_legacy($b['a']));
}
