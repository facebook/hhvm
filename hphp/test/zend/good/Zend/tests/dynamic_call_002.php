<?hh
<<__EntryPoint>> function main(): void {
$a = new stdClass;

HH\dynamic_class_meth(HH\get_class_from_object($a), $a)();
}
