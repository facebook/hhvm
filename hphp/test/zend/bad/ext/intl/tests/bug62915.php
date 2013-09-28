<?php

class foo extends IntlTimeZone {
		public $foo = 'test';
			
				public function __construct() { }
}

$x = new foo;

try {
		$z = clone $x;
} catch (Exception $e) {
		var_dump($e->getMessage());
}