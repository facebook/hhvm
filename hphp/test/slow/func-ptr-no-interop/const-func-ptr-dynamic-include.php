<?hh

class A {
  public static $arr = dict[
    'foo' => fun('foo'),
    'meth' => class_meth(Cls::class, 'meth'),
  ];

  const arr2 = dict[
    'foo' => fun('foo'),
    'meth' => class_meth(Cls::class, 'meth'),
  ];
}

<<__EntryPoint>>
function main_constant_functions() {
  $count = __hhvm_intrinsics\apc_fetch_no_check('count');
  if ($count === false) {
     $count = 0;
  }
  if ($count < 2) {
    ++$count;
    apc_store('count', $count);

    $path = __DIR__.'/const-func-ptr-include'.$count.'.inc';
    include $path;

    echo "===== run with include$count ======\n";

    var_dump((A::$arr['foo'])());
    var_dump((A::$arr['meth'])());
    var_dump((A::arr2['foo'])());
    var_dump((A::arr2['meth'])());
  }
}
