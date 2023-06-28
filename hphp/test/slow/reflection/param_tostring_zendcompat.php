<?hh

// Check output exactly matches PHP - some mocking frameworks use this, as
// Zend doesn't provide $param->getTypeText() or $param->getTypehintText()
//
// For non-null defaults, see param_tostring_hh.php

class MyObject {}

class Foo {
  function bar(
    $untyped,
    mixed $m,
    string $str,
    int $i,
    MyObject $o,
    this $thiz,
    $opt_untyped = null,
    mixed $opt_m = null,
    MyObject $opt_o = null
  ) :mixed{
  }
}


<<__EntryPoint>>
function main_param_tostring_zendcompat() :mixed{
$method = (new ReflectionClass('Foo'))->getMethod('bar');

var_dump(
  array_map(
    function($param) {
      return $param->__toString();
    },
    $method->getParameters()
  )
);
}
