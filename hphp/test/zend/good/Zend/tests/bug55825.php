<?php
trait T1 {

  private static $incX =1;
	public function inc() {
		echo self::$incX++ . "\n";
	}
}
class C { use T1; }
$c1 = new C;
$c1->inc();
$c1->inc();
