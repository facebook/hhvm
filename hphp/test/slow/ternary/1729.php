<?php

function f() {
}
 function g() {
}
 $t = true;
$a = $t ? f() : g();
var_dump($a);
