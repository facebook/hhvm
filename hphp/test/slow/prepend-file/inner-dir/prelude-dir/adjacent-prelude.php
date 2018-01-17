<?hh

echo "In file\n";
var_dump(class_exists("Foo"));

class Y extends Foo {}
var_dump((new Y)->x);
