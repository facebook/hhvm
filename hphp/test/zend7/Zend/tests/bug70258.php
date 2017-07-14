<?php
class A {
	public $arr;
	public function core() {
		$this->arr["no_pack"] = 1;
		while (1) {
			$this->arr[] = 1;
		}
	}
}

$a = new A;
$a->core();
?>
