<?hh

async function test(
  X::T $x,
  ?Vector<Z::Y::X> $v,
): Awaitable<A::B::C> {}


<<__EntryPoint>>
function main_type_constant6() :mixed{
$reflect = new ReflectionFunction('test');

var_dump($reflect->getReturnTypeText());

foreach($reflect->getParameters() as $param) {
   var_dump('$'.$param->getName().' : '.$param->getTypeText());
}
}
