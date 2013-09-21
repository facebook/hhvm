<?php
class UsesMagic extends ArrayObject {
	function __get($n)     {  echo "In " . __METHOD__ . "!\n"; }
	function __set($n, $v) {  echo "In " . __METHOD__ . "!\n"; }
	function __isset($n)   {  echo "In " . __METHOD__ . "!\n"; }
	function __unset($n)   {  echo "In " . __METHOD__ . "!\n"; }
}
$ao = new UsesMagic(array(), ArrayObject::ARRAY_AS_PROPS);

echo "Doesn't trigger __get.\n";
echo $ao->prop1;

echo "Doesn't trigger __set.\n";    
$ao->prop2 = 'foo';

echo "Doesn't trigger __unset.\n";
unset($ao->prop3);

echo "Shouldn't trigger __isset.\n";
isset($ao->prop4);
?>