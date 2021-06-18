<?hh

// Source php weakref extension
<<__EntryPoint>>
function main_weakref_acquire() {
$r = new stdClass;
$wr1 = new WeakRef($r);
var_dump($wr1->acquire());
__hhvm_intrinsics\launder_value($r);
unset($r);
}
