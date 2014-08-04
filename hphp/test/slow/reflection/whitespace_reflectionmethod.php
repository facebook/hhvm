<?php

class Example {
  function herp($derp) {}
  function foo($bar, $baz) {}
}

var_dump((string) (new ReflectionMethod('Example::herp')));
var_dump((string) (new ReflectionMethod('Example::foo')));
