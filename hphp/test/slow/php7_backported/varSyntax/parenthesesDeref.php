<?php

<<__EntryPoint>>
function main_parentheses_deref() {
$array = [&$array, 1];
var_dump(($array)[1]);
var_dump((($array[0][0])[0])[1]);
var_dump(((object) ['a' => 0, 'b' => 1])->b);
$obj = (object) ['a' => 0, 'b' => ['var_dump', 1]];
(clone $obj)->b[0](1);
}
