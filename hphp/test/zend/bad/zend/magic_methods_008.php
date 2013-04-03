<?php

abstract class b {
	abstract function __set($a, $b);
}

class a extends b {
	private function __set($a, $b) {
	}
}

?>