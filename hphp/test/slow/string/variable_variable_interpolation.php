<?php

<<__EntryPoint>>
function main_variable_variable_interpolation() {
$foo = 'hello';
$bar = 'foo';
echo "{$$bar}\n";
echo $$bar."\n";
}
