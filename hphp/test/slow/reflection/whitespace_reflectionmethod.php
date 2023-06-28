<?hh

class Example {
  function herp($derp) :mixed{}
  function foo($bar, $baz) :mixed{}
}


<<__EntryPoint>>
function main_whitespace_reflectionmethod() :mixed{
var_dump((string) (new ReflectionMethod('Example::herp')));
var_dump((string) (new ReflectionMethod('Example::foo')));
}
