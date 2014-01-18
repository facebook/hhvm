<?php

// Check output exactly matches PHP - some mocking frameworks use this, as
// Zend doesn't provide $param->getTypeText() or $param->getTypehintText()
//
// For non-null defaults, see param_tostring_hh.php

class Foo {
  function bar(
    $untyped,
    mixed $m,
    string $str,
    int $i,
    Object $o,
    self $self,
    $opt_untyped = null,
    mixed $opt_m = null,
    Object $opt_o = null
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
