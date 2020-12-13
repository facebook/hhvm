<?hh

<<__EntryPoint>>
function main() {
  $a = varray[17];
  unset($a[0]);
  apc_add('a', vec[$a]);

  $b = HH\array_mark_legacy(varray[17]);
  unset($b[0]);
  apc_add('b', vec[$b]);

  $success = null;
  $a = apc_fetch('a', inout $success)[0];
  var_dump(HH\is_array_marked_legacy($a));
  print(HH\get_provenance($a)."\n");

  $success = null;
  $b = apc_fetch('b', inout $success)[0];
  var_dump(HH\is_array_marked_legacy($b));
  print(HH\get_provenance($b)."\n");
}
