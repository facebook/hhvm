<?hh

// Source php weakref extension
<<__EntryPoint>>
function main_weakref_multiple() :mixed{
$r = new stdClass;
$wr1 = new WeakRef($r);
var_dump($wr1->valid());
__hhvm_intrinsics\launder_value($wr1);
$wr1 = null;
$wr2 = new WeakRef($r);
var_dump($wr2->valid());
__hhvm_intrinsics\launder_value($wr2);
$wr2 = null;
__hhvm_intrinsics\launder_value($r);
}
