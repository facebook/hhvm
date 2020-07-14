<?hh

<<__EntryPoint>>
function main() {
  $v = __hhvm_intrinsics\launder_value(varray[]);
  $d = darray($v);

  var_dump(HH\get_provenance($v));
  var_dump(HH\get_provenance($d));
}
