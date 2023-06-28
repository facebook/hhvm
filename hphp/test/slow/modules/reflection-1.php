<?hh

<<file:__EnableUnstableFeatures('modules')>>
module B;

class C {
  function f() :mixed{}
}

<<__EntryPoint>>
function main() :mixed{
  include 'reflection-1.inc';
  include 'reflection-1.inc2';
  var_dump((new ReflectionFunction('f'))->getModule());
  var_dump((new ReflectionFunction('g'))->getModule());
  var_dump((new ReflectionMethod('C', 'f'))->getModule());
  var_dump((new ReflectionClass('C'))->getModule());
}
