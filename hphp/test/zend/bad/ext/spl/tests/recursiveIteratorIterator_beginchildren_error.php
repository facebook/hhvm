<?php 

$arr = array(array(1,2),2);
$arrOb = new ArrayObject($arr);

$recArrIt = new RecursiveArrayIterator($arrOb->getIterator());

class MyRecursiveIteratorIterator extends RecursiveIteratorIterator {
    
    function beginchildren() {
    	throw new Exception;
    }
}


$recItIt = new MyRecursiveIteratorIterator($recArrIt, RecursiveIteratorIterator::LEAVES_ONLY, RecursiveIteratorIterator::CATCH_GET_CHILD);

var_dump($recItIt->next());

$recItIt2 = new MyRecursiveIteratorIterator($recArrIt, RecursiveIteratorIterator::LEAVES_ONLY);

var_dump($recItIt2->next());

?>