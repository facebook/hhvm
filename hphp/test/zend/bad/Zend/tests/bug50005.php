<?php

class a extends exception {
	public function __construct() {
		$this->file = null;
	}
}

throw new a;

?>