<?php
$n = new MongoId('4f06e55e44670ab92b000000');
var_export($n);
echo "\n";

$a = MongoId::__set_state(array(
   '$id' => '4f06e55e44670ab92b000000',
));
var_dump($a);
?>
