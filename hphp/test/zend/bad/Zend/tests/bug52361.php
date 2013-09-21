<?php
class aaa {
	public function __destruct() {
		try {
			throw new Exception(__CLASS__);
		} catch(Exception $ex) {
			echo "1. $ex\n";
		}
	}
}
function bbb() {
	$a = new aaa();
	throw new Exception(__FUNCTION__);
}
try {
	bbb();
	echo "must be skipped !!!";
} catch(Exception $ex) {
	echo "2. $ex\n";
}
?>