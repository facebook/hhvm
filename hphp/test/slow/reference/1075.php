<?hh

function f(inout $a) {
 $a = 'ok';
}

 <<__EntryPoint>>
function main_1075() {
$a = 10;
 f(inout $a);
 var_dump($a);
}
