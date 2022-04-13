<?hh

<<file:__EnableUnstableFeatures("modules")>>

<<Ok, Yes>>
new module A {}
new module B {}

<<__EntryPoint>>
function main() {
  $m = new ReflectionModule('A');
  var_dump($m->getName());
  var_dump($m->getAttributes());

  $m = new ReflectionModule('B');
  var_dump($m->getName());
  var_dump($m->getAttributes());

  $m = new ReflectionModule('C');
}
