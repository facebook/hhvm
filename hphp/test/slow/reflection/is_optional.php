<?hh

function test1($a, $b=10, $c, ...$params) :mixed{}
function test2($a=10, ...$params) :mixed{}


<<__EntryPoint>>
function main_is_optional() :mixed{
foreach(vec['test1', 'test2'] as $func) {
  $reflect = new ReflectionFunction($func);
  foreach($reflect->getParameters() as $p) {
    var_dump($p->isOptional());
  }
}
}
