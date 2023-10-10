<?hh

function regular($a, $b, $c) :mixed{
  echo '* ', __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

function variadic($a, ...$args) :mixed{
  echo '* ', __FUNCTION__, "\n";
  var_dump($a, $args);
}

interface I {
  public function regular($a, $b, $c):mixed;
  public function variadic($a, ...$args):mixed;
}

class C implements I {
  public function __construct($a, $b, $c) {
    echo '* ', __METHOD__, "\n";
    var_dump($a, $b, $c);
  }

  public static function stRegular($a, $b, $c) :mixed{
    echo '* ', __METHOD__, "\n";
    var_dump($a, $b, $c);
  }

  public function regular($a, $b, $c) :mixed{
    echo '* ', __METHOD__, "\n";
    var_dump($a, $b, $c);
  }

  public static function stVariadic($a, ...$args) :mixed{
    echo '* ', __METHOD__, "\n";
    var_dump($a, $args);
  }

  public function variadic($a, ...$args) :mixed{
    echo '* ', __METHOD__, "\n";
    var_dump($a, $args);
  }
}

class D implements I {
  public function regular($a, $b, $c) :mixed{}
  public function variadic($a, ...$args) :mixed{}
}

function test_call_array_equivalent($args) :mixed{
  echo "= ", __FUNCTION__, " =", "\n";
  var_dump($args);
  echo "\n";

  regular(...$args);
  variadic(...$args);
  C::stRegular(...$args);
  C::stVariadic(...$args);
  $inst = new C(...$args);
  $inst->regular(...$args);
  $inst->variadic(...$args);
  echo "\n";
}

function variadic_with_func_get_args(...$args) :mixed{
  echo '* ', __FUNCTION__, "\n";
  var_dump($args);
}

/* TODO(t4599363): support multiple unpacks
function test_call_array_equivalent_multi($args) {
  echo "= ", __FUNCTION__, " =", "\n";
  var_dump($args);
  echo "\n";

  regular(...$args, ...$args);
  variadic(...$args, ...$args);
  C::stRegular(...$args, ...$args);
  C::stVariadic(...$args, ...$args);
  $inst = new C(...$args, ...$args);
  $inst->regular(...$args, ...$args);
  $inst->variadic(...$args, ...$args);
  echo "\n";
} */

function test_param_mix($args) :mixed{
  echo "= ", __FUNCTION__, " =", "\n";
  var_dump($args);
  echo "\n";

  $prefix = 'passed regularly';
  regular($prefix, ...$args);
  variadic($prefix, ...$args);
  C::stRegular($prefix, ...$args);
  C::stVariadic($prefix, ...$args);
  $inst = new C($prefix, ...$args);
  $inst->regular($prefix, ...$args);
  $inst->variadic($prefix, ...$args);
  echo "\n";

  $prefix2 = 'also passed regularly';
  $prefix3 = 'arg that ensures more args passed than declared';
  variadic($prefix, $prefix2, $prefix3, ...$args);
  variadic_with_func_get_args($prefix, $prefix2, ...$args);
  regular($prefix, $prefix2, ...$args);
  regular($prefix, $prefix2, ...$args);
  variadic($prefix, $prefix2, ...$args);
  C::stRegular($prefix, $prefix2, ...$args);
  C::stVariadic($prefix, $prefix2, ...$args);
  $inst = new C($prefix, $prefix2, ...$args);
  $inst->regular($prefix, $prefix2, ...$args);
  $inst->variadic($prefix, $prefix2, ...$args);
  echo "\n";
}

function test_param_mix_typed(varray $args, I $iface) :mixed{
  echo "= ", __FUNCTION__, " =", "\n";
  var_dump($args);
  echo "\n";

  $prefix = 'passed regularly';
  regular($prefix, ...$args);
  variadic($prefix, ...$args);
  C::stRegular($prefix, ...$args);
  C::stVariadic($prefix, ...$args);
  $inst = new C($prefix, ...$args);
  $inst->regular($prefix, ...$args);
  $inst->variadic($prefix, ...$args);
  $iface->regular($prefix, ...$args);
  $iface->variadic($prefix, ...$args);
  echo "\n";

  $prefix2 = 'also passed regularly';
  $prefix3 = 'arg that ensures more args passed than declared';
  variadic($prefix, $prefix2, $prefix3, ...$args);
  variadic_with_func_get_args($prefix, $prefix2, ...$args);
  regular($prefix, $prefix2, ...$args);
  regular($prefix, $prefix2, ...$args);
  variadic($prefix, $prefix2, ...$args);
  C::stRegular($prefix, $prefix2, ...$args);
  C::stVariadic($prefix, $prefix2, ...$args);
  $inst = new C($prefix, $prefix2, ...$args);
  $inst->regular($prefix, $prefix2, ...$args);
  $inst->variadic($prefix, $prefix2, ...$args);
  $iface->regular($prefix, $prefix2, ...$args);
  $iface->variadic($prefix, $prefix2, ...$args);
  echo "\n";
}

function main() :mixed{
  $a = varray['a', 'b', 'c'];
  $v = Vector {'a', 'b', 'c'};
  // TODO(t4599379): arbitrary traversables
  // $t = new ArrayIterator(['a', 'b', 'c']);

  test_call_array_equivalent($a);
  test_call_array_equivalent($v);
  // test_call_array_equivalent($t);

  // TODO(t4599363): support multiple unpacks
  // test_call_array_equivalent_twice($a);
  // test_call_array_equivalent_twice($v);

  test_param_mix($a);
  test_param_mix($v);
  // test_param_mix($t);

  test_param_mix_typed($a, new C('a', 'b', 'c'));

  echo "Done\n";
}


<<__EntryPoint>>
function main_unpack_call() :mixed{
main();
}
