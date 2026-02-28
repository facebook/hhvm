<?hh



<<__EntryPoint>>
function main_1825() :mixed{
$f = function($a) {
 return $a;
 }
;
 var_dump($f('x'));
 apc_store('key', $f);
 $g = __hhvm_intrinsics\apc_fetch_no_check('key');
 var_dump($g);
}
