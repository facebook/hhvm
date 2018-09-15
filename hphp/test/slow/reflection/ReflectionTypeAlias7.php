<?hh

type MyType = void;


<<__EntryPoint>>
function main_reflection_type_alias7() {
$x = new ReflectionTypeAlias('mytype');
echo $x->getName(), "\n";
}
