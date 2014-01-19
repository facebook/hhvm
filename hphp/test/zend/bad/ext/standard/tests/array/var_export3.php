<?php
class kake {
	public $mann;
	protected $kvinne;

	function __construct()
	{
		$this->mann = 42;
		$this->kvinne = 43;
	}
}

$kake = new kake;

var_export($kake);
?>