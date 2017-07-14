<?php

set_error_handler(function() {
    $obj = new stdClass;
    $obj->test = 'meow';
    return true;
});
 
$a = new stdClass;
$a->undefined .= 'test';
var_dump($a);

?>
