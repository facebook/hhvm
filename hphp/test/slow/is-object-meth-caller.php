<?hh

class C { <<__DynamicallyCallable>> function f() :mixed{} }

<<__EntryPoint>>
function main() :mixed{
  $mc = meth_caller(C::class, 'f');
  $lv = __hhvm_intrinsics\launder_value($mc);

  $dc = HH\dynamic_meth_caller(C::class, 'f');
  $l2 = __hhvm_intrinsics\launder_value($dc);

  $f = is_object<>;
  $g = __hhvm_intrinsics\launder_value($f);

  var_dump(is_object($mc));
  var_dump(is_object($lv));
  var_dump(is_object($dc));
  var_dump(is_object($l2));

  var_dump($f($mc));
  var_dump($f($lv));
  var_dump($f($dc));
  var_dump($f($l2));

  var_dump($g($mc));
  var_dump($g($lv));
  var_dump($g($dc));
  var_dump($g($l2));
}
