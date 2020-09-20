<?hh
function foo($a) {
 print $a;
}
 function bar($a) {
 return $a;
}


<<__EntryPoint>>
function main_1711() {
error_reporting(E_ALL & ~E_NOTICE);
 foo('ok', bar('bad'));
}
