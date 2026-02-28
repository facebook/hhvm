<?hh

class C {}

function test_varray(string $name, varray $array) :mixed{
  print("\n=============================\nTesting: $name\n");
  var_dump(HH\is_array_marked_legacy($array));
  $array = HH\array_mark_legacy($array);
  var_dump(HH\is_array_marked_legacy($array));
  $array = vec($array);
  var_dump(HH\is_array_marked_legacy($array));
  $array = varray($array);
  var_dump(HH\is_array_marked_legacy($array));
}

function test_darray(string $name, darray $array) :mixed{
  print("\n=============================\nTesting: $name\n");
  var_dump(HH\is_array_marked_legacy($array));
  $array = HH\array_mark_legacy($array);
  var_dump(HH\is_array_marked_legacy($array));
  $array = dict($array);
  var_dump(HH\is_array_marked_legacy($array));
  $array = darray($array);
  var_dump(HH\is_array_marked_legacy($array));
}

<<__EntryPoint>>
function main() :mixed{
  test_varray('vec[]', vec[]);
  test_varray('vec[new C()]', vec[new C()]);
  test_darray('dict[]', dict[]);
  test_darray('dict[17 => new C()]', dict[17 => new C()]);

  print("\n=============================\nTesting: static marked varray\n");
  $v = HH\array_mark_legacy(vec[]);
  var_dump(HH\is_array_marked_legacy($v));
  var_dump(HH\is_array_marked_legacy(varray($v)));
  var_dump(HH\is_array_marked_legacy(darray($v)));
  var_dump(HH\is_array_marked_legacy(vec($v)));
  var_dump(HH\is_array_marked_legacy(dict($v)));

  print("\n=============================\nTesting: static marked darray\n");
  $d = HH\array_mark_legacy(dict[]);
  var_dump(HH\is_array_marked_legacy($d));
  var_dump(HH\is_array_marked_legacy(varray($d)));
  var_dump(HH\is_array_marked_legacy(darray($d)));
  var_dump(HH\is_array_marked_legacy(vec($d)));
  var_dump(HH\is_array_marked_legacy(dict($d)));
}
