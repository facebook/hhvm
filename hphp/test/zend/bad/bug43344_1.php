<?php
namespace Foo;
function f1($a=bar) {
	return $a;
}
function f2($a=array(bar)) {
	return $a[0];
}
function f3($a=array(bar=>0)) {
	reset($a);
	return key($a);
}
echo bar."\n";
echo f1()."\n";
echo f2()."\n";
echo f3()."\n";
?>