<?hh

function handle_error($_no, $str, ...) {
  $matches = null;

  $pat1 = '/Implicit clsmeth to ([^ ]+) conversion/';
  if (preg_match_with_matches($pat1, $str, inout $matches)) {
    echo "** CAST: clsmeth -> {$matches[1]}\n";
    return true;
  }
  $pat2 =
    '/class_meth Compat: Argument ([0-9]+) passed to [^()]+\(\) must be of '.
    'type ([^,]+), clsmeth given/';
  if (preg_match_with_matches($pat2, $str, inout $matches)) {
    echo "** CAST ARG({$matches[1]}): clsmeth -> {$matches[2]}\n";
    return true;
  }
  $pat3 =
    '/class_meth Compat: Value returned from [^()]+\(\) must be of '.
    'type ([^,]+), clsmeth given/';
  if (preg_match_with_matches($pat3, $str, inout $matches)) {
    echo "** CAST RET: clsmeth -> {$matches[1]}\n";
    return true;
  }
  return false;
}

class Foo { static function bar() {} }

function LV($x) { return __hhvm_intrinsics\launder_value($x); }

function args(
  array $arr,
  varray $varr,
  Traversable $trav,
  Container $cont,
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

function ret_varr(): varray { return class_meth(Foo::class, 'bar'); }
function ret_arr(): array   { return class_meth(Foo::class, 'bar'); }

function ret_varr_dyn(): varray { return LV(class_meth(Foo::class, 'bar')); }
function ret_arr_dyn(): array   { return LV(class_meth(Foo::class, 'bar')); }

function inout_args(
  inout array $arr,
  inout varray $varr,
  inout Traversable $trav,
  inout Container $cont,
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
  args(
    class_meth(Foo::class, 'bar'),
    class_meth(Foo::class, 'bar'),
    class_meth(Foo::class, 'bar'),
    class_meth(Foo::class, 'bar'),
  );

  var_dump(ret_varr());
  var_dump(ret_arr());

  $io1 = class_meth(Foo::class, 'bar');
  $io2 = class_meth(Foo::class, 'bar');
  $io3 = class_meth(Foo::class, 'bar');
  $io4 = class_meth(Foo::class, 'bar');

  inout_args(inout $io1, inout $io2, inout $io3, inout $io4);
  var_dump($io1, $io2, $io3, $io4);

  $foo = class_meth(Foo::class, 'bar');
  var_dump((array)$foo);
  var_dump(varray($foo));

  var_dump(array_map($x ==> var_dump($x) ?? 42, $foo));
}

function test_dynamic() {
  args(
    LV(class_meth(Foo::class, 'bar')),
    LV(class_meth(Foo::class, 'bar')),
    LV(class_meth(Foo::class, 'bar')),
    LV(class_meth(Foo::class, 'bar')),
  );

  var_dump(ret_varr_dyn());
  var_dump(ret_arr_dyn());

  $io1 = LV(class_meth(Foo::class, 'bar'));
  $io2 = LV(class_meth(Foo::class, 'bar'));
  $io3 = LV(class_meth(Foo::class, 'bar'));
  $io4 = LV(class_meth(Foo::class, 'bar'));

  inout_args(inout $io1, inout $io2, inout $io3, inout $io4);
  var_dump($io1, $io2, $io3, $io4);

  $foo = LV(class_meth(Foo::class, 'bar'));
  var_dump((array)$foo);
  var_dump(varray($foo));

  var_dump(array_map($x ==> var_dump($x) ?? 42, $foo));
}

<<__EntryPoint>>
function main() {
  set_error_handler('handle_error');

  test_static();  test_static();  test_static();
  test_dynamic(); test_dynamic(); test_dynamic();
}
