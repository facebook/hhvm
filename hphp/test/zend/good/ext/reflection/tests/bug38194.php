<?hh
class MyObject { }
<<__EntryPoint>> function main(): void {
$objectClass= new ReflectionClass('MyObject');
var_dump($objectClass->isSubclassOf($objectClass));
}
