<?php
error_reporting(E_ALL);

trait Counter {

  private static $incC = 0;
   public function inc() {
     self::$incC = self::$incC + 1;
     $c = self::$incC;
     echo "$c\n";
   }
}


class C1 {
   use Counter;
}

$o = new C1();
$o->inc();
$o->inc();

