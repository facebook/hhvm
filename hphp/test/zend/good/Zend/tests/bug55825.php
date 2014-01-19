<?php
trait T1 {
	public function inc() {
		static $x=1;
		echo $x++ . "\n";
	}
}
class C { use T1; }
$c1 = new C;
$c1->inc();
$c1->inc();