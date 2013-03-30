<?php

$a = function(&$a) { $a = 1; };

$a($a);

?>