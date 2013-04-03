<?php

$a = function () use(&$a) {};
var_dump($a);

?>
===DONE===