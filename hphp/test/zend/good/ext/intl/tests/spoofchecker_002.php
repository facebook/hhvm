<?php

$url = "http://www.payp\xD0\xB0l.com";

$x = new Spoofchecker();
echo "Checking if words are confusable\n";
var_dump($x->areConfusable("hello, world", "goodbye, world"));
var_dump($x->areConfusable("hello, world", "hello, world"));
var_dump($x->areConfusable("hello, world", "he11o, wor1d"));
?>
