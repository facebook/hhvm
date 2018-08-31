<?php


<<__EntryPoint>>
function main_1567() {
var_dump(array_filter(array(1, 1003, 34, 5006), create_function('$x', 'return '.'$x > 1000;
')));
}
