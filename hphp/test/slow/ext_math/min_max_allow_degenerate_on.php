<?php


<<__EntryPoint>>
function main_min_max_allow_degenerate_on() {
var_dump(min(5));
var_dump(min(array()));
var_dump(max(5));
var_dump(max(array()));
}
