<?php
$ar = new SplFixedArray(1);
echo "size: ".$ar->getSize()."\n";
$ar->setSize((PHP_INT_SIZE==8)?0x2000000000000001:0x40000001);
echo "size: ".$ar->getSize()."\n";
?>
