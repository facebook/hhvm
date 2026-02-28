<?hh


<<__EntryPoint>>
function main_reflection_type_alias9() :mixed{
require_once 'ReflectionTypeAlias.inc';

$x = new ReflectionTypeAlias('MyType');
var_dump($x->getFileName());

$x = new ReflectionTypeAlias('MyOpaqueType');
var_dump($x->getFileName());
}
