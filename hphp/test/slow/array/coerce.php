<?php


<<__EntryPoint>>
function main_coerce() {
var_dump(Locale::lookup(new stdclass, 'foo'));
var_dump(Locale::lookup(STDIN, 'foo'));
}
