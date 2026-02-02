<?hh

function foo(int $bar, AnyArray $baz) :mixed{
}
function herp(\HH\int $derp) :mixed{
}
function bar<T as arraykey as int>(T $a) : mixed{
  return "foo";
}

<<__EntryPoint>>
function main_hack() :mixed{
;

$rp = new ReflectionParameter('foo', 'bar');
var_dump($rp->getClass());
var_dump($rp->getTypehintText());

$rp = new ReflectionParameter('foo', 'baz');
var_dump($rp->getClass());
var_dump($rp->getTypehintText());

herp(123);
$rp = new ReflectionParameter('herp', 'derp');
var_dump($rp->getClass());
var_dump($rp->getTypehintText());

$rp = new ReflectionParameter('bar', 'a');
var_dump($rp->getClass());
var_dump($rp->getTypehintText());
}
