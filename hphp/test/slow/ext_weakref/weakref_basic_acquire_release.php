<?hh

// Source php weakref extension
<<__EntryPoint>>
function main_weakref_basic_acquire_release() :mixed{
$o = new stdClass;
$wr = new WeakRef($o);
$wr->acquire();
$wr->acquire();
var_dump($wr->valid(), $wr->get());
__hhvm_intrinsics\launder_value($o);
unset($o);
$wr->release();
var_dump($wr->valid(), $wr->get());
$wr->release();
var_dump($wr->valid(), $wr->get());
}
