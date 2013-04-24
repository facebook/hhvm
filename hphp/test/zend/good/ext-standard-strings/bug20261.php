<?php
 $first = "boo";
 $second = $first;
 $rot = "";

 echo "1: ".$first."\n";
 echo "2: ".$second."\n";
 echo "3: ".$rot."\n";

 $rot = str_rot13($second);

 echo "1: ".$first."\n";
 echo "2: ".$second."\n";
 echo "3: ".$rot."\n";
?>