<?php
$data = array(1 => 'val1',
              array(2 => 'val2',
                    array(3 => 'val3'),
                   ),
              4 => 'val4'
             );

$iterator = new RecursiveIteratorIterator(new
RecursiveArrayIterator($data));
foreach($iterator as $foo) {
    $key = $iterator->key();
    echo "update $key\n";
    var_dump($iterator->getInnerIterator());
    $iterator->offsetSet($key, 'alter');
    var_dump($iterator->getInnerIterator());
}
$copy = $iterator->getArrayCopy();
var_dump($copy);
?>