<?hh

type MyType = void;


<<__EntryPoint>>
function main_reflection_type_alias7() :mixed{
$x = new ReflectionTypeAlias('MyType');
echo $x->getName(), "\n";
}
