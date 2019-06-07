<?hh

class Example {
  function herp($derp) {}
  function foo($bar, $baz) {}
}


<<__EntryPoint>>
function main_whitespace_reflectionmethod() {
var_dump((string) (new ReflectionMethod('Example::herp')));
var_dump((string) (new ReflectionMethod('Example::foo')));
}
