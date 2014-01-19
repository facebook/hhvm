<?php

class Bar {
}

$b = new Bar;

var_dump($b == NULL);
var_dump($b != NULL);
var_dump($b == true);
var_dump($b != true);
var_dump($b == false);
var_dump($b != false);
var_dump($b == "");
var_dump($b != "");
var_dump($b == 0);
var_dump($b != 0);
var_dump($b == 1);
var_dump($b != 1);
var_dump($b == 1.0);
var_dump($b != 1.0);
var_dump($b == 1);


echo "Done\n";
?>