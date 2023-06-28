<?hh

// Source php weakref extension
<<__EntryPoint>>
function main_weakref_basic() :mixed{
$o = new stdClass;
$wr = new WeakRef($o);
var_dump($wr->valid(), $wr->get());
__hhvm_intrinsics\launder_value($o);
unset($o);
var_dump($wr->valid(), $wr->get());
}
