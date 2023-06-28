<?hh

function foo(int $bar, AnyArray $baz) :mixed{
}
<<__EntryPoint>>
function main_php_forcehh() :mixed{
;

$rp = new ReflectionParameter('foo', 'bar');
var_dump($rp->getClass());
var_dump($rp->getTypehintText());

$rp = new ReflectionParameter('foo', 'baz');
var_dump($rp->getClass());
var_dump($rp->getTypehintText());
}
