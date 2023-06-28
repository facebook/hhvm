<?hh
class foo {
        public $x = "bar";
}

<<__EntryPoint>>
function main_unserialize_error_001() :mixed{
$z = varray[new foo(), 2, "3"];
$s = serialize($z);

var_dump(unserialize($s, darray["allowed_classes" => null]));
var_dump(unserialize($s, darray["allowed_classes" => 0]));
var_dump(unserialize($s, darray["allowed_classes" => 1]));
}
