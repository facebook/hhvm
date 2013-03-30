<?php

interface d {
	static function B();	
}

interface c {
	function b();	
}

class_alias('c', 'w');

interface a extends d, w { }

?>