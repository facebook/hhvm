<?hh

<<file:__EnableUnstableFeatures('modules'), __Module("B")>>

class C {
  function f() {}
}

<<__EntryPoint>>
function main() {
  include 'reflection-1.inc';
  include 'reflection-1.inc2';
  var_dump((new ReflectionFunction('f'))->getModule());
  var_dump((new ReflectionFunction('g'))->getModule());
  var_dump((new ReflectionMethod('C', 'f'))->getModule());
}
