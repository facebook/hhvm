<?hh

<<__EntryPoint>>
function main() {
  include 'reflection-7.inc';

  $m = new ReflectionModule('a');
  var_dump($m->getExports());
  var_dump($m->getImports());
}
