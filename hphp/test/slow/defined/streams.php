<?php


// NB: These will return false in server mode
<<__EntryPoint>>
function main_streams() {
var_dump(defined('STDIN'));
var_dump(defined('STDOUT'));
var_dump(defined('STDERR'));
}
