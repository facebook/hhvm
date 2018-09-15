<?php


<<__EntryPoint>>
function main_bad_array_filter() {
error_reporting(-1);
var_dump(array_filter(array(1,2,3), 'fizzle'));
}
