<?php

echo "An object is in an array and is referenced. As expected, var_dumping the array shows '&':\n";
$a = array(new stdclass);
$b =& $a[0];
var_dump($a);

echo "Naturally, this remains true if we modify the object:\n";
$a[0]->x = 1;
var_dump($a);


echo "\n\nObtain the array of interfaces implemented by C.\n";
interface I {}
class C implements I {}
$rc = new ReflectionClass('C');
$a = $rc->getInterfaces();
echo "The result is an array in which each element is an object (an instance of ReflectionClass)\n";
echo "Var_dumping this array shows that the elements are referenced. By what?\n";
var_dump($a);

echo "Modify the object, and it is apparently no longer referenced.\n";
$a['I']->x = 1;
var_dump($a);

?>
