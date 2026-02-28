<?hh
class foo {
        public $x = "bar";
}
<<__EntryPoint>> function main(): void {
$z = vec[new foo(), 2, "3"];
$s = serialize($z);

var_dump(unserialize($s));
var_dump(unserialize($s, dict["allowed_classes" => false]));
var_dump(unserialize($s, dict["allowed_classes" => true]));
var_dump(unserialize($s, dict["allowed_classes" => vec["bar"]]));
var_dump(unserialize($s, dict["allowed_classes" => vec["foo"]]));
var_dump(unserialize($s, dict["allowed_classes" => vec["bar", "foo"]]));
}
