<?hh

// Source php weakref extension
<<__EntryPoint>>
function main_weakref_acquire_clone() {
$r = new stdClass;
$wr1 = new WeakRef($r);
var_dump($wr1->acquire());
__hhvm_intrinsics\launder_value($r);
unset($r);
$wr2 = clone $wr1;
var_dump($wr1->release());
var_dump($wr2->valid());
}
