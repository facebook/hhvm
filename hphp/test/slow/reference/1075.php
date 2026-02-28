<?hh

function f(inout $a) :mixed{
 $a = 'ok';
}

 <<__EntryPoint>>
function main_1075() :mixed{
$a = 10;
 f(inout $a);
 var_dump($a);
}
