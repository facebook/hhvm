<?hh

function expected_class_union((string | int) $class_name) : void {
 /* HH_FIXME[4026] */
 $x = new $class_name();
}
