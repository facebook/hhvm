<?php

function test(&$a, $b) {
 $a = 'ok';
}

 <<__EntryPoint>>
function main_1183() {
$a = 'Test';
 $a(&$a, 10);
 print $a;
}
