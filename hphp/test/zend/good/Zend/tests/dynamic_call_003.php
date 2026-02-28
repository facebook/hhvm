<?hh
<<__EntryPoint>> function main(): void {
$a = new stdClass;
$b = 1;

HH\dynamic_class_meth(HH\get_class_from_object($a), $b)();
}
