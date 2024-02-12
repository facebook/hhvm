<?hh

class Y extends Foo {}


<<__EntryPoint>>
function main_sub_prelude() :mixed{
echo "In file\n";
var_dump(class_exists("Foo"));
var_dump((new Y)->x);
}
