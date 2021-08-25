<?hh

class C {}

function expected_class_intersection((string & classname<C>) $class_name) : void {
 $x = new $class_name();
}
