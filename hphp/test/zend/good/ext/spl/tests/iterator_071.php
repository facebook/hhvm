<?hh 

class MyRecursiveIteratorIterator extends RecursiveIteratorIterator {
    
    function nextelement() {
    	echo __METHOD__."\n";
    }
}

<<__EntryPoint>>
function main_entry(): void {

  $arr = array(array(1,2),2);
  $arrOb = new ArrayObject($arr);

  $recArrIt = new RecursiveArrayIterator($arrOb->getIterator());


  $recItIt = new MyRecursiveIteratorIterator($recArrIt, RecursiveIteratorIterator::CHILD_FIRST);

  foreach ($recItIt as $key => $val) echo "$key\n";
}
