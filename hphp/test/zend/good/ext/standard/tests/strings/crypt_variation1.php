<?php

$b = str_repeat("A", 124);
echo crypt("A", "$5$" . $b)."\n";
$b = str_repeat("A", 125);
echo crypt("A", "$5$" . $b)."\n";
$b = str_repeat("A", 4096);
echo crypt("A", "$5$" . $b)."\n";

?>