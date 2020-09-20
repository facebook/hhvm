<?hh
class foo {
        public $x = "bar";
}
<<__EntryPoint>> function main(): void {
$z = varray[new foo(), 2, "3"];
$s = serialize($z);

var_dump(unserialize($s));
var_dump(unserialize($s, darray["allowed_classes" => false]));
var_dump(unserialize($s, darray["allowed_classes" => true]));
var_dump(unserialize($s, darray["allowed_classes" => varray["bar"]]));
var_dump(unserialize($s, darray["allowed_classes" => varray["FOO"]]));
var_dump(unserialize($s, darray["allowed_classes" => varray["bar", "foO"]]));
}
