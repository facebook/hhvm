<?php
class FIIFO {

	public function __construct() {
		$this->x = "x";
		throw new Exception;
	}

}

class hariCow extends FIIFO {

	public function __construct() {
		try {
			parent::__construct();
		} catch(Exception $e) {
		}
		$this->y = "y";
		try {
			$this->z = new FIIFO;
		} catch(Exception $e) {
		}
	}
	
	public function __toString() {
		return "Rusticus in asino sedet.";
	}

}

try {
	$db = new FIIFO();
} catch(Exception $e) {
}
var_dump($db);

$db = new hariCow;

var_dump($db);
?>
===DONE===