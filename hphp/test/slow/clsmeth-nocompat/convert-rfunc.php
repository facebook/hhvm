<?hh

function handle_error($_no, $msg, ...) {
  $matches = null;
  $pat1 = '/Argument ([0-9]+) passed to ([^(]+)\(\) must be an instance of ([^,]+), clsmeth given/';
  if (preg_match_with_matches($pat1, $msg, inout $matches)) {
    echo "[TYPE VIOLATION] ARG {$matches[1]} of {$matches[2]} ".
      "(wanted: {$matches[3]}, got: clsmeth)\n";
    throw new Exception;
  }
  $pat2 = '/Value returned from function ([^(]+)\(\) must be of type ([^,]+), clsmeth given/';
  if (preg_match_with_matches($pat2, $msg, inout $matches)) {
    echo "[TYPE VIOLATION] RET {$matches[1]} ".
      "(wanted: {$matches[2]}, got: clsmeth)\n";
    throw new Exception;
  }
  if ($msg === 'array_map(): Argument #2 should be an array or collection') {
    echo "[NOTICE] $msg\n";
    return true;
  }
  return false;
}

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
  try {
    args<AnyArray, varray, Traversable, Container>(
      class_meth(Foo::class, 'bar'),
      class_meth(Foo::class, 'bar'),
      class_meth(Foo::class, 'bar'),
      class_meth(Foo::class, 'bar'),
    );
  } catch (Exception $_) {}

  try { var_dump(ret_varr<varray>()); } catch (Exception $_) {}
  try { var_dump(ret_arr<AnyArray>()); } catch (Exception $_) {}

  $io1 = class_meth(Foo::class, 'bar');
  $io2 = class_meth(Foo::class, 'bar');
  $io3 = class_meth(Foo::class, 'bar');
  $io4 = class_meth(Foo::class, 'bar');

  try {
    inout_args<AnyArray, varray, Traversable, Container>(
      inout $io1,
      inout $io2,
      inout $io3,
      inout $io4
    );
  } catch (Exception $_) {}
  var_dump($io1, $io2, $io3, $io4);

  $foo = class_meth(Foo::class, 'bar');
  try { var_dump(varray($foo)); } catch (Exception $_) {}

  try {
    var_dump(array_map($x ==> var_dump($x) ?? 42, $foo));
  } catch (Exception $_) {}
}

function test_dynamic() {
  try {
    args<AnyArray, varray, Traversable, Container>(
      LV(class_meth(Foo::class, 'bar')),
      LV(class_meth(Foo::class, 'bar')),
      LV(class_meth(Foo::class, 'bar')),
      LV(class_meth(Foo::class, 'bar')),
    );
  } catch (Exception $_) {}

  try { var_dump(ret_varr_dyn<varray>()); } catch (Exception $_) {}
  try { var_dump(ret_arr_dyn<AnyArray>()); } catch (Exception $_) {}

  $io1 = LV(class_meth(Foo::class, 'bar'));
  $io2 = LV(class_meth(Foo::class, 'bar'));
  $io3 = LV(class_meth(Foo::class, 'bar'));
  $io4 = LV(class_meth(Foo::class, 'bar'));

  try {
    inout_args<AnyArray, varray, Traversable, Container>(
      inout $io1,
      inout $io2,
      inout $io3,
      inout $io4
    );
  } catch (Exception $_) {}
  var_dump($io1, $io2, $io3, $io4);

  $foo = LV(class_meth(Foo::class, 'bar'));
  try { var_dump(varray($foo)); } catch (Exception $_) {}

  try {
    var_dump(array_map($x ==> var_dump($x) ?? 42, $foo));
  } catch (Exception $_) {}
}

<<__EntryPoint>>
function main() {
  set_error_handler(handle_error<>);

  test_static();  test_static();  test_static();
  test_dynamic(); test_dynamic(); test_dynamic();
}
