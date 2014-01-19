<?php

if(0){
class y{
}
}
else{
class y{
}
}
abstract class x extends y {
  private static $nextSerial = 1;
  private $serial = 0;
  public function __construct() {
    $this->serial = self::$nextSerial++;
  }
}
