<?hh

class C {}

function expected_class_intersection((string & class<C>) $class_name) : void {
 $x = new $class_name();
}
