<?php
function errh($errno, $errstr) {
	var_dump($errstr);
}
set_error_handler("errh");

interface a{}
class b implements a { function f($a=1) {}}
class c extends b {function f() {}}
?>