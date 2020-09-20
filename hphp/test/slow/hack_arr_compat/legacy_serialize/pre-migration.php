<?hh

<<__EntryPoint>>
function main() {
  $a = __hhvm_intrinsics\launder_value(varray[1, 2, 3]);
  $b = __hhvm_intrinsics\launder_value(darray["bing" => "crosby"]);

  $a[] = 4;
  $b[] = 42;

  $a = \HH\array_mark_legacy($a);
  $b = \HH\array_mark_legacy($b);

  var_dump(\HH\is_array_marked_legacy($a));
  var_dump(\HH\is_array_marked_legacy($b));

  # these should raise warnings
  var_dump(\HH\array_mark_legacy(vec[]));
  var_dump(\HH\array_mark_legacy(dict[]));
  var_dump(\HH\is_array_marked_legacy(keyset[]));
}
