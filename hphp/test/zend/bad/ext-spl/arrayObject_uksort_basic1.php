<?php
/* Prototype  : int ArrayObject::uksort(callback cmp_function)
 * Description: proto int ArrayIterator::uksort(callback cmp_function)
 * Sort the entries by key using user defined function. 
 * Source code: ext/spl/spl_array.c
 * Alias to functions: 
 */

echo "*** Testing ArrayObject::uksort() : basic functionality ***\n";
// Reverse sorter
function cmp($value1, $value2) {
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 < $value2) {
    return 1;
  }
  else
    return -1;
}
$ao = new ArrayObject(array(3=>0, 2=>1, 5=>2, 6=>3, 1=>4));

$ao->uksort('cmp');
var_dump($ao);
?>
===DONE===