<?hh
class C {}

function expected_class_union_classname((int | classname<C>) $class_name) : void {
 /* HH_FIXME[4026] */
 $x = new $class_name();
}
