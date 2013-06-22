<?php
$i= DateInterval::createFromDateString('2 days');
var_dump($i);
echo $i->format("%d"), "\n";
echo $i->format("%a"), "\n";
?>