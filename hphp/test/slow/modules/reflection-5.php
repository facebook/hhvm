<?hh

<<file:__EnableUnstableFeatures('modules')>>

<<__EntryPoint>>
function main() {
  include 'reflection-5.inc';

  $m = new ReflectionModule('A.B');
  var_dump($m->getDocComment());
}
