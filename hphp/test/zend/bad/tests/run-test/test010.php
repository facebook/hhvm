<?php
var_dump(stream_get_contents(STDIN));
var_dump(stream_get_contents(fopen('php://stdin', 'r')));
var_dump(file_get_contents('php://stdin'));
?>