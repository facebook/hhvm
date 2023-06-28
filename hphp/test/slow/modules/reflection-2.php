<?hh



<<Ok, Yes>>
new module A {}

<<__EntryPoint>>
function main() :mixed{
  include "reflection-2.inc";

  // Same file
  $m = new ReflectionModule('A');
  var_dump($m->getName());
  var_dump($m->getAttributes());

  // Different file
  $m = new ReflectionModule('B');
  var_dump($m->getName());
  var_dump($m->getAttributes());

  // Doesn't exist
  $m = new ReflectionModule('C');
}
