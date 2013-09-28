<?php
class SplMinHeap2 extends SplMinHeap {
	public function testCompare1() {
		return parent::compare();
	}
	public function testCompare2() {
		return parent::compare(1);
	}
	public function testCompare3() {
		return parent::compare(1, 2, 3);
	}
}

$h = new SplMinHeap2();
$h->testCompare1();
$h->testCompare2();
$h->testCompare3();
?>