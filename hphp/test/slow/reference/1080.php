<?hh

function f(inout $a) {
 $a = 'ok';
}
 class T {
}

 <<__EntryPoint>>
function main_1080() {
$a = new T();
 $a->b = 10;
 $__b = $a->b;
 f(inout $__b);
 $a->b = $__b;
 var_dump($a);
}
