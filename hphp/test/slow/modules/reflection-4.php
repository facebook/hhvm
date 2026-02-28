<?hh

<<__EntryPoint>>
function main() :mixed{
  include 'reflection-4.inc';
  include 'reflection-4.inc2';

  var_dump((new ReflectionMethod('C', 'f'))->getModule());

  $m = new ReflectionModule('A.B');
  var_dump($m->getName());
}
