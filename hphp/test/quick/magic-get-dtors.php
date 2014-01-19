<?php

class c{
  public static $idx = 0;
  private $x;
  public function __construct() {
    $this->x = self::$idx++;
    printf("c %d constructing\n", $this->x);
  }
  public function __destruct() {
    printf("c %d destructing\n", $this->x);
  }
  public function __get($name) {
    return new C();
  }
}

function main() {
  $o = new c();
  $v = $o->fakeprop->fakeprop->fakeprop->fakeprop->otherfakeprop;
  $w = $o->fakeprop; // Exercise the translator's CGetM <H P>
  echo "Returning from main\n";
}
echo "Starting\n";
main();
echo "Done\n";
