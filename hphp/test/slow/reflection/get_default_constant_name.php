<?hh

const ONE = 1;
const TWO = ONE + ONE;

function params($one = ONE, $four = TWO * TWO, $ten = TWO * 5) :mixed{
  return $one + $four + $ten;
}

function test($param) :mixed{
  $r = new ReflectionParameter('params', $param);

  var_dump($r->getDefaultValue());
  var_dump($r->getDefaultValueText());
  var_dump($r->getDefaultValueConstantName());
}


<<__EntryPoint>>
function main_get_default_constant_name() :mixed{
test('one');
test('four');
test('ten');
}
