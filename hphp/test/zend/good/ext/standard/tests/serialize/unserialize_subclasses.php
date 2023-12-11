<?hh

class C {}
class D extends C {}
<<__EntryPoint>> function main(): void {
$c = serialize(new C);
$d = serialize(new D);

var_dump(unserialize($c, dict["allowed_classes" => vec["C"]]));
var_dump(unserialize($c, dict["allowed_classes" => vec["D"]]));
var_dump(unserialize($d, dict["allowed_classes" => vec["C"]]));
var_dump(unserialize($d, dict["allowed_classes" => vec["D"]]));
}
