<?php

class Callbacks {
  public $count = 0;
  public function filter($n) {
    $this->count++;
    return $n % 2;
  }

  public static $scount = 0;
  public static function sfilter($n) {
    self::$scount++;
    return $n % 3;
  }
}

function main() {
  $a = array(1,2,3,4,5,6,7,8);
  $cb = new Callbacks();
  var_dump(array_filter($a, array($cb, 'filter')));
  echo $cb->count . " times\n";

  var_dump(array_filter($a, array('Callbacks', 'sfilter')));
  echo Callbacks::$scount . " times\n";
}
main();
