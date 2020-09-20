<?hh

<<__EntryPoint>>
function main() {
  $darr = __hhvm_intrinsics\launder_value(
    darray['trash' => rand()]
  );
  $varr = __hhvm_intrinsics\launder_value(
    varray[rand()]
  );
  $dict = __hhvm_intrinsics\launder_value(
    dict['trash' => rand()]
  );
  $vec = __hhvm_intrinsics\launder_value(
    vec[rand()]
  );

  $bad_darr = __hhvm_intrinsics\launder_value(
    darray[0 => rand()]
  );
  $bad_dict = __hhvm_intrinsics\launder_value(
    dict[0 => rand()]
  );

  $stuffs = vec[$darr, $varr, $dict, $vec, $bad_darr, $bad_dict];

  foreach ($stuffs as $potato) {
    var_dump($potato is (int));
    var_dump($potato is shape('trash' => int));
  }
}
