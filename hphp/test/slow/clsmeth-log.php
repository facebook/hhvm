<?hh

class Foo { static function bar() {} }

<<__EntryPoint>>
function main() {
  $cma = class_meth(Foo::class, 'bar');
  $arra = darray['a' => dict['r' => class_meth(Foo::class, 'bar')], 'b' => 123];
  var_dump(serialize($cma));
  var_dump(serialize($arra));
  var_dump(json_encode($cma));
  var_dump(json_encode($arra));

  $cmb = __hhvm_intrinsics\launder_value($cma);
  $arrb = __hhvm_intrinsics\launder_value($arra);
  var_dump(serialize($cmb));
  var_dump(serialize($arrb));
  var_dump(json_encode($cmb));
  var_dump(json_encode($arrb));
}
