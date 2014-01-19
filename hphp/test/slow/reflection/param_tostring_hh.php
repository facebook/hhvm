<?hh

// Zend doesn't understand 'string' and 'int' typehints, and will only allow
// NULL defaults. These are in a separate test so that Zend can still generate
// param_tostring_zendcompat.php.expect

class Foo {
  function bar(
    string $str = 'baz',
    int $i = 42,
  ) {
  }
}

$method = (new ReflectionClass('Foo'))->getMethod('bar');

var_dump(
  array_map(
    function($param) {
      return $param->__toString();
    },
    $method->getParameters()
  )
);
