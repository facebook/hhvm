<?php

$a = 1;
$fn = function ($a) use ($a) {
    var_dump($a);
};
$fn(2);

?>
