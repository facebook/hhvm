<?php
$stack = new SplStack();

$stack[] = "var1";
$stack[] = "var2";
$stack[] = "var3";

foreach ($stack as $var) {
    echo $var . "\n";
}
