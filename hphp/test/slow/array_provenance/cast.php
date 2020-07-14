<?hh

function foo($a) {
  return varray($a);
}

function bar($b) {
  return darray($b);
}


<<__EntryPoint>>
function main() {
  var_dump(
    HH\get_provenance(
      foo(__hhvm_intrinsics\launder_value(vec[1, 2, 3]))));
  var_dump(
    HH\get_provenance(
      bar(__hhvm_intrinsics\launder_value(dict["42" => "bar"]))));

  $a = __hhvm_intrinsics\launder_value(
    vec[rand()]
  );
  $a = foo($a);
  var_dump(HH\get_provenance($a));

  $a = __hhvm_intrinsics\launder_value(
    varray[rand()]
  );
  $a = foo($a);
  var_dump(HH\get_provenance($a));

  $b = __hhvm_intrinsics\launder_value(
    dict['foo' => rand()]
  );
  $b = bar($b);
  var_dump(HH\get_provenance($b));

  $b = __hhvm_intrinsics\launder_value(
    darray['foo' => rand()]
  );
  $b = bar($b);
  var_dump(HH\get_provenance($b));
}
