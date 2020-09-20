<?hh

class Foo {
}


<<__EntryPoint>>
function main_reflectionclass_serializable() {
$rc = new ReflectionClass(Foo::class);
$serialized = serialize($rc);
var_dump(json_encode($serialized));
var_dump(unserialize($serialized)->getName());
}
