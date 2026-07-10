<?hh

class Example {
  function herp($derp) :mixed{}
  function foo($bar, $baz) :mixed{}
}


<<__EntryPoint>>
function main_whitespace_reflectionmethod() :mixed{
var_dump((new ReflectionMethod('Example::herp'))->__toString());
var_dump((new ReflectionMethod('Example::foo'))->__toString());
}
