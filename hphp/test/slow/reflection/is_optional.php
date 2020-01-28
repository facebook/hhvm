<?hh

function test1($a, $b=10, $c, ...$params) {}
function test2($a=10, ...$params) {}


<<__EntryPoint>>
function main_is_optional() {
foreach(varray['test1', 'test2'] as $func) {
  $reflect = new ReflectionFunction($func);
  foreach($reflect->getParameters() as $p) {
    var_dump($p->isOptional());
  }
}
}
