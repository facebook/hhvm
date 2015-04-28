<?hh

function regular($a, $b, $c) {
  echo '* ', __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

function variadic($a, ...$args) {
  echo '* ', __FUNCTION__, "\n";
  var_dump($a, $args);
}

class C {
  public function __construct($a, $b, $c) {
    echo '* ', __METHOD__, "\n";
    var_dump($a, $b, $c);
  }

  public static function __callStatic($name, $args) {
    echo '* ', __METHOD__, ' for ', $name, "\n";
    var_dump($args);
  }

  public function __call($name, $args) {
    echo '* ', __METHOD__, ' for ', $name, "\n";
    var_dump($args);
  }

  public static function stRegular($a, $b, $c) {
    echo '* ', __METHOD__, "\n";
    var_dump($a, $b, $c);
  }

  public function regular($a, $b, $c) {
    echo '* ', __METHOD__, "\n";
    var_dump($a, $b, $c);
  }

  public static function stVariadic($a, ...$args) {
    echo '* ', __METHOD__, "\n";
    var_dump($a, $args);
  }

  public function variadic($a, ...$args) {
    echo '* ', __METHOD__, "\n";
    var_dump($a, $args);
  }
}

function test_call_array_equivalent($args) {
  echo "= ", __FUNCTION__, " =", "\n";
  var_dump($args);
  echo "\n";

  regular(...$args);
  variadic(...$args);
  C::stRegular(...$args);
  C::stVariadic(...$args);
  C::stMagic(...$args);
  $inst = new C(...$args);
  $inst->regular(...$args);
  $inst->variadic(...$args);
  $inst->magic(...$args);
  echo "\n";
}

function variadic_with_func_get_args(...$args) {
  echo '* ', __FUNCTION__, "\n";
  var_dump(func_get_args());
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

class dtor {
  function __destruct() {
    echo "dtor::__destruct\n";
  }
}

function test_param_mix($args) {
  echo "= ", __FUNCTION__, " =", "\n";
  var_dump($args);
  echo "\n";

  $prefix = 'passed regularly';
  regular($prefix, ...$args);
  variadic($prefix, ...$args);
  C::stRegular($prefix, ...$args);
  C::stVariadic($prefix, ...$args);
  C::stMagic($prefix, ...$args);
  $inst = new C($prefix, ...$args);
  $inst->regular($prefix, ...$args);
  $inst->variadic($prefix, ...$args);
  $inst->magic($prefix, ...$args);
  echo "\n";

  $prefix2 = 'also passed regularly';
  $prefix3 = 'arg that ensures more args passed than declared';
  variadic($prefix, $prefix2, $prefix3, ...$args);
  variadic_with_func_get_args($prefix, $prefix2, ...$args);
  variadic_with_func_get_args(new dtor, new dtor, ...[new dtor]);
  echo "-- after destruct\n";
  regular($prefix, $prefix2, ...$args);
  regular($prefix, $prefix2, ...$args);
  variadic($prefix, $prefix2, ...$args);
  C::stRegular($prefix, $prefix2, ...$args);
  C::stVariadic($prefix, $prefix2, ...$args);
  C::stMagic($prefix, $prefix2, ...$args);
  $inst = new C($prefix, $prefix2, ...$args);
  $inst->regular($prefix, $prefix2, ...$args);
  $inst->variadic($prefix, $prefix2, ...$args);
  $inst->magic($prefix, $prefix2, ...$args);
  echo "\n";
}

function main() {
  $a = array('a', 'b', 'c');
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

  echo "Done\n";
}

main();
