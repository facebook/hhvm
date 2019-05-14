<?hh

function append($a) {
  $a = __hhvm_intrinsics\launder_value($a);
  $a[] = "hello";
  var_dump($a);
  var_dump(HH\get_provenance($a));
}

function setelem_int($a) {
  $a = __hhvm_intrinsics\launder_value($a);
  $a[42] = "hello";
  var_dump($a);
  var_dump(HH\get_provenance($a));
}

function setelem_string($a) {
  $a = __hhvm_intrinsics\launder_value($a);
  $a["blargh!"] = "goodbye";
  var_dump($a);
  var_dump(HH\get_provenance($a));
}

function setelem_invalid($a) {
  $a = __hhvm_intrinsics\launder_value($a);
  $a[false] = "eek";
  var_dump($a);
  var_dump(HH\get_provenance($a));
}

function elemd_int($a) {
  $a = __hhvm_intrinsics\launder_value($a);
  $a[0] += 5;
  var_dump($a);
  var_dump(HH\get_provenance($a));
}

<<__EntryPoint>>
function main() {
  append(vec[]);
  append(dict[]);
  setelem_int(dict[]);
  setelem_string(dict[]);
  elemd_int(dict[0 => 10]);
  elemd_int(vec[10]);

  $a = __hhvm_intrinsics\launder_value(dict[]);
  $a["foo"] = dict[];
  $a["foo"]["bar"] = 42;
  var_dump(HH\get_provenance($a));
  var_dump(HH\get_provenance($a["foo"]));

  $b = __hhvm_intrinsics\launder_value(dict[42 => dict[]]);
  $b[42][42] = "fasdf";
  var_dump(HH\get_provenance($b));
  var_dump(HH\get_provenance($b[42]));
}
