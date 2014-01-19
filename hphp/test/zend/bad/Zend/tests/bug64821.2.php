<?php

class a extends exception {
	public function __construct() {
		$this->line = array();
	}
}

throw new a;

?>