<?php
$a=array("1","2");
$b=&$a;
foreach($a as $i){
    echo $i;
    foreach($a as $p);
}
echo "\n";
?>