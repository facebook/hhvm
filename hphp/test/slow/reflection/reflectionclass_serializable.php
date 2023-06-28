<?hh

class Foo {
}


<<__EntryPoint>>
function main_reflectionclass_serializable() :mixed{
$rc = new ReflectionClass(Foo::class);
$serialized = serialize($rc);
var_dump(json_encode($serialized));
var_dump(unserialize($serialized)->getName());
}
