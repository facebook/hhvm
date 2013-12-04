<?php
class foo {
	function cb($param) {
		var_dump($param);
		return "yes!";
	}
}

var_dump(preg_replace('', array(), ''));
var_dump(preg_replace_callback("/(ab)(cd)(e)/", array(new foo(), "cb"), 'abcde'));
?>