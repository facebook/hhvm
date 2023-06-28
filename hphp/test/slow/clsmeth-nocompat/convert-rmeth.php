<?hh

function handle_error($_no, $msg, ...) :mixed{
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

class Foo { static function bar() :mixed{} }

function LV($x) :mixed{ return __hhvm_intrinsics\launder_value($x); }

class R<reify Tarr, reify Tvar, reify Ttrav, reify Tcont> {
  function args(
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

  function ret_varr(): Tvar { return Foo::bar<>; }
  function ret_arr():  Tarr { return Foo::bar<>; }

  function ret_varr_dyn(): Tvar {return LV(Foo::bar<>); }
  function ret_arr_dyn():  Tarr {return LV(Foo::bar<>); }

  function inout_args(
    inout Tarr $arr,
    inout Tvar $varr,
    inout Ttrav $trav,
    inout Tcont $cont,
  ) :mixed{
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
}

function test_static() :mixed{
  $R = new R<AnyArray, varray, Traversable, Container>();

  try {
    $R->args(
      Foo::bar<>,
      Foo::bar<>,
      Foo::bar<>,
      Foo::bar<>,
    );
  } catch (Exception $_) {}

  try { var_dump($R->ret_varr()); } catch (Exception $_) {}
  try { var_dump($R->ret_arr()); } catch (Exception $_) {}

  $io1 = Foo::bar<>;
  $io2 = Foo::bar<>;
  $io3 = Foo::bar<>;
  $io4 = Foo::bar<>;

  try {
    $R->inout_args(
      inout $io1,
      inout $io2,
      inout $io3,
      inout $io4
    );
  } catch (Exception $_) {}
  var_dump($io1, $io2, $io3, $io4);

  $foo = Foo::bar<>;
  try { var_dump(varray($foo)); } catch (Exception $_) {}

  try {
    var_dump(array_map($x ==> var_dump($x) ?? 42, $foo));
  } catch (Exception $_) {}
}

function test_dynamic() :mixed{
  $R = new R<AnyArray, varray, Traversable, Container>();

  try {
    $R->args(
      LV(Foo::bar<>),
      LV(Foo::bar<>),
      LV(Foo::bar<>),
      LV(Foo::bar<>),
    );
  } catch (Exception $_) {}

  try { var_dump($R->ret_varr_dyn()); } catch (Exception $_) {}
  try { var_dump($R->ret_arr_dyn()); } catch (Exception $_) {}

  $io1 = LV(Foo::bar<>);
  $io2 = LV(Foo::bar<>);
  $io3 = LV(Foo::bar<>);
  $io4 = LV(Foo::bar<>);

  try {
    $R->inout_args(
      inout $io1,
      inout $io2,
      inout $io3,
      inout $io4
    );
  } catch (Exception $_) {}
  var_dump($io1, $io2, $io3, $io4);

  $foo = LV(Foo::bar<>);
  try { var_dump(varray($foo)); } catch (Exception $_) {}

  try {
    var_dump(array_map($x ==> var_dump($x) ?? 42, $foo));
  } catch (Exception $_) {}
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handle_error<>);

  test_static();  test_static();  test_static();
  test_dynamic(); test_dynamic(); test_dynamic();
}
