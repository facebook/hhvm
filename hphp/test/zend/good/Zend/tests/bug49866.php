<?php
$a = "string";
$b = &$a[1];
$b = "f";
echo $a;