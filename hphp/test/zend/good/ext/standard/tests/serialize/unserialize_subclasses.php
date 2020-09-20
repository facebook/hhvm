<?hh

class C {}
class D extends C {}
<<__EntryPoint>> function main(): void {
$c = serialize(new C);
$d = serialize(new D);

var_dump(unserialize($c, darray["allowed_classes" => varray["C"]]));
var_dump(unserialize($c, darray["allowed_classes" => varray["D"]]));
var_dump(unserialize($d, darray["allowed_classes" => varray["C"]]));
var_dump(unserialize($d, darray["allowed_classes" => varray["D"]]));
}
