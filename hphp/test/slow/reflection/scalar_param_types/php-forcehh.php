<?hh

function foo(int $bar, AnyArray $baz) {
}
<<__EntryPoint>>
function main_php_forcehh() {
;

$rp = new ReflectionParameter('foo', 'bar');
var_dump($rp->getClass());
var_dump($rp->getTypehintText());

$rp = new ReflectionParameter('foo', 'baz');
var_dump($rp->getClass());
var_dump($rp->getTypehintText());
}
