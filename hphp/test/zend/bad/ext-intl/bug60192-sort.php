<?php

class Collator2 extends Collator{
	public function __construct() {
		// ommitting parent::__construct($someLocale);
	}
}

$c = new Collator2();
$a = array('a', 'b');
$c->sort($a);