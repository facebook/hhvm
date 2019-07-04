<?hh

require_once 'ReflectionTypeAlias.inc';
type MyType = Vector<int>;
newtype MyOpaqueType = (function (string, Set<int>): void);
<<__EntryPoint>> function main(): void {
$x = new ReflectionTypeAlias('MyType');
var_dump($x->getFileName());

$x = new ReflectionTypeAlias('MyOpaqueType');
var_dump($x->getFileName());
}
