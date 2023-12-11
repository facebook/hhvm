<?hh

class X {
}

function test($a) :mixed{
  apc_store('foo', dict[1 => $a]);
  $a = __hhvm_intrinsics\apc_fetch_no_check('foo');
  $a[1] = 'bar';
  var_dump($a);

  $a = __hhvm_intrinsics\apc_fetch_no_check('foo');
  $a["1"] = 'bar';
  var_dump($a);

  $a = __hhvm_intrinsics\apc_fetch_no_check('foo');
  foreach ($a as $k => $x) {
    var_dump($x);
  }

  $a = __hhvm_intrinsics\apc_fetch_no_check('foo');
  $a[1] = 'bar';
  foreach ($a as $k => $x) {
    var_dump($x);
  }
}
<<__EntryPoint>> function main(): void {
test(42);
}
