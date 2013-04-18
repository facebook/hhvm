<?php
$i=new DateInterval('P7D');
var_dump($i);
echo $i->format("%d"), "\n";
echo $i->format("%a"), "\n";
?>