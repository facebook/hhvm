<?php
class foo {}
function no_typehint($a) {
	var_dump($a);
}
function typehint(foo $a) {
	var_dump($a);
}
function no_typehint_ref(&$a) {
	var_dump($a);
}
function typehint_ref(foo &$a) {
	var_dump($a);
}
$v = new foo();
$a = array(new foo(), 1, 2);
no_typehint($v);
typehint($v);
no_typehint_ref($v);
typehint_ref($v);
echo "===no_typehint===\n";
array_walk($a, 'no_typehint');
echo "===no_typehint_ref===\n";
array_walk($a, 'no_typehint_ref');
echo "===typehint===\n";
array_walk($a, 'typehint');
echo "===typehint_ref===\n";
array_walk($a, 'typehint_ref');
?>