<?php

class a extends exception {
	public function __construct() {
		$this->line = array();
		$this->file = NULL;
	}
}

throw new a;

?>