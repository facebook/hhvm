<?hh

class Foo { static function bar() {} }

function LV($x) { return __hhvm_intrinsics\launder_value($x); }

function args<reify Tarr, reify Tvar, reify Ttrav, reify Tcont>(
  Tarr $arr,
  Tvar $varr,
  Ttrav $trav,
  Tcont $cont,
): void {
  var_dump(
    $arr,
    $varr,
    $trav,
    $cont
  );
  if ($arr[0] !== Foo::class  || $arr[1] !== 'bar')  echo "Fail!\n";
  if ($varr[0] !== Foo::class || $varr[1] !== 'bar') echo "Fail!\n";
  if ($trav[0] !== Foo::class || $trav[1] !== 'bar') echo "Fail!\n";
  if ($cont[0] !== Foo::class || $cont[1] !== 'bar') echo "Fail!\n";
}

function ret_varr<reify T>(): T { return class_meth(Foo::class, 'bar'); }
function ret_arr<reify T>():  T { return class_meth(Foo::class, 'bar'); }

function ret_varr_dyn<reify T>(): T {return LV(class_meth(Foo::class, 'bar')); }
function ret_arr_dyn<reify T>():  T {return LV(class_meth(Foo::class, 'bar')); }

function inout_args<reify Tarr, reify Tvar, reify Ttrav, reify Tcont>(
  inout Tarr $arr,
  inout Tvar $varr,
  inout Ttrav $trav,
  inout Tcont $cont,
) {
  var_dump(
    $arr,
    $varr,
    $trav,
    $cont
  );
  if ($arr[0] !== Foo::class  || $arr[1] !== 'bar')  echo "Fail!\n";
  if ($varr[0] !== Foo::class || $varr[1] !== 'bar') echo "Fail!\n";
  if ($trav[0] !== Foo::class || $trav[1] !== 'bar') echo "Fail!\n";
  if ($cont[0] !== Foo::class || $cont[1] !== 'bar') echo "Fail!\n";
}

function test_static() {
  args<array, varray, Traversable, Container>(
    class_meth(Foo::class, 'bar'),
    class_meth(Foo::class, 'bar'),
    class_meth(Foo::class, 'bar'),
    class_meth(Foo::class, 'bar'),
  );

  var_dump(ret_varr<varray>());
  var_dump(ret_arr<array>());

  $io1 = class_meth(Foo::class, 'bar');
  $io2 = class_meth(Foo::class, 'bar');
  $io3 = class_meth(Foo::class, 'bar');
  $io4 = class_meth(Foo::class, 'bar');

  inout_args<array, varray, Traversable, Container>(
    inout $io1,
    inout $io2,
    inout $io3,
    inout $io4
  );
  var_dump($io1, $io2, $io3, $io4);

  $foo = class_meth(Foo::class, 'bar');
  var_dump((array)$foo);
  var_dump(varray($foo));

  var_dump(array_map($x ==> var_dump($x) ?? 42, $foo));
}

function test_dynamic() {
  args<array, varray, Traversable, Container>(
    LV(class_meth(Foo::class, 'bar')),
    LV(class_meth(Foo::class, 'bar')),
    LV(class_meth(Foo::class, 'bar')),
    LV(class_meth(Foo::class, 'bar')),
  );

  var_dump(ret_varr_dyn<varray>());
  var_dump(ret_arr_dyn<array>());

  $io1 = LV(class_meth(Foo::class, 'bar'));
  $io2 = LV(class_meth(Foo::class, 'bar'));
  $io3 = LV(class_meth(Foo::class, 'bar'));
  $io4 = LV(class_meth(Foo::class, 'bar'));

  inout_args<array, varray, Traversable, Container>(
    inout $io1,
    inout $io2,
    inout $io3,
    inout $io4
  );
  var_dump($io1, $io2, $io3, $io4);

  $foo = LV(class_meth(Foo::class, 'bar'));
  var_dump((array)$foo);
  var_dump(varray($foo));

  var_dump(array_map($x ==> var_dump($x) ?? 42, $foo));
}

<<__EntryPoint>>
function main() {
  test_static();  test_static();  test_static();
  test_dynamic(); test_dynamic(); test_dynamic();
}
