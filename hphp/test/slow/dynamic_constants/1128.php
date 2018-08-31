<?php

function foo() {
 global $g;
 return $g ? -1 : 15;
}

 <<__EntryPoint>>
function main_1128() {
var_dump(TEST);
 define('TEST', foo());
 var_dump(TEST);
}
