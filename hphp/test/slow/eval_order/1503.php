<?hh

class X {
}
 function foo() :mixed{
 var_dump('foo');
}


 <<__EntryPoint>>
function main_1503() :mixed{
$x = new X;
 unset($x->a[foo()]->y);
}
