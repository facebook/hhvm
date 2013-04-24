<?php
class a {
	static function b() {
		return true;
	}
}
$a = new a();
$res = call_user_method('b', $a);
var_dump($res);
?>