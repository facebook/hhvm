<?hh

class X {
}
 function foo() {
 var_dump('foo');
}


 <<__EntryPoint>>
function main_1503() {
$x = new X;
 unset($x->a[foo()]->y);
}
