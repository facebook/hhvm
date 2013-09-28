<?php

$array = array('1' => 'one',
               '2' => 'two',
               '3' => 'three');

$object = new ArrayObject($array);
$object[] = 'four';

$arrayObject = new ArrayObject($object);

$arrayObject[] = 'five';

var_dump($arrayObject);

?>
===DONE===
<?php exit(0); ?>