<?php 

$arr = array(array(1,2));
$arrOb = new ArrayObject($arr);

$recArrIt = new RecursiveArrayIterator($arrOb->getIterator());

class MyRecursiveIteratorIterator extends RecursiveIteratorIterator {
    
    function endchildren() {
    	throw new Exception;
    }
}


$recItIt = new MyRecursiveIteratorIterator($recArrIt, RecursiveIteratorIterator::LEAVES_ONLY, RecursiveIteratorIterator::CATCH_GET_CHILD);

foreach ($recItIt as $val) echo "$val\n";

$recItIt2 = new MyRecursiveIteratorIterator($recArrIt, RecursiveIteratorIterator::LEAVES_ONLY);

echo "===NEXT LOOP===\n";

foreach ($recItIt2 as $val) echo "$val\n";

?>