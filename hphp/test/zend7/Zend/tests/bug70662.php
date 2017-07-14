<?php

$a = [];
set_error_handler(function() use(&$a) {
    $a['b'] = 2;
});
$a['b'] += 1;
var_dump($a);

?>
