<?hh
function foo($a) :mixed{
 print $a;
}
 function bar($a) :mixed{
 return $a;
}


<<__EntryPoint>>
function main_1711() :mixed{
error_reporting(E_ALL & ~E_NOTICE);
 foo('ok', bar('bad'));
}
