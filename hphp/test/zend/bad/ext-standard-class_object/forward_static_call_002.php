<?php

class A
{
	public static function test() {
		echo "A\n";
	}
}

function test() {
	forward_static_call(array('A', 'test'));
}

test();

?>