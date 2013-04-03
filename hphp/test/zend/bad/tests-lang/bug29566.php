<?php
$var="This is a string";

$dummy="";
unset($dummy);

foreach($var['nosuchkey'] as $v) {
}
?>
===DONE===