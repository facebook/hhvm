<?hh

class X {
}

<<__EntryPoint>>
function main_1376() {
;
$x = new X;
$x->a = 1;
$x->b = 'hello';
$x->c = $x;
var_dump(http_build_query($x));
}
