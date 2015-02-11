<?hh

async function test(
  X::T $x,
  ?Vector<Z::Y::X<int, string>> $v,
): Awaitable<A::B::C> {}

$reflect = new ReflectionFunction('test');

var_dump($reflect->getReturnTypeText());

foreach($reflect->getParameters() as $param) {
   var_dump('$'.$param->getName().' : '.$param->getTypeText());
}
