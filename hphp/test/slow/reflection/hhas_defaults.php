<?hh

function test() :mixed{
  $x = new ReflectionFunction('array_filter');
  $params = $x->getParameters();
  $p1 = $params[1];
  var_dump($p1->getDefaultValueText());
}


<<__EntryPoint>>
function main_hhas_defaults() :mixed{
test();
}
