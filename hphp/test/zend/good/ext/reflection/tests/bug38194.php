<?hh
class Object { }
<<__EntryPoint>> function main(): void {
$objectClass= new ReflectionClass('Object');
var_dump($objectClass->isSubclassOf($objectClass));
}
