<?php

class a extends exception {
	public function __construct() {
		$this->message = NULL;
		$this->string  = NULL;
		$this->code    = array();
		$this->line = "hello";
	}
}

throw new a;

?>