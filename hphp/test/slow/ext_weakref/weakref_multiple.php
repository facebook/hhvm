<?hh

// Source php weakref extension
<<__EntryPoint>>
function main_weakref_multiple() :mixed{
$r = new stdClass;
$wr1 = new WeakRef($r);
var_dump($wr1->valid());
__hhvm_intrinsics\launder_value($wr1);
unset($wr1);
$wr2 = new WeakRef($r);
var_dump($wr2->valid());
__hhvm_intrinsics\launder_value($wr2);
unset($wr2);
__hhvm_intrinsics\launder_value($r);
}
