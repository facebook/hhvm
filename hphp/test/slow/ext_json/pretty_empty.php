<?php


<<__EntryPoint>>
function main_pretty_empty() {
var_dump(json_encode(array(), JSON_PRETTY_PRINT));
var_dump(json_encode(new stdClass, JSON_PRETTY_PRINT));
}
