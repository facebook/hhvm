<?php
$a = new stdClass();

$array = array(array('z',$a),array('q','s'));

$rai = new RecursiveArrayIterator($array,RecursiveArrayIterator::CHILD_ARRAYS_ONLY);

foreach (new RecursiveIteratorIterator($rai) as $t) {
    var_dump($t);
}
echo "Second:\n";
$rai = new RecursiveArrayIterator($array);
foreach (new RecursiveIteratorIterator($rai) as $t) {
    var_dump($t);
}
?>