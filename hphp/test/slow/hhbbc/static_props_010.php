<?php

class y { public $bar; }
class x {
  private $bar = "asd";
  private static $set = 12;
  private static $hm = "asd";
  private static $heh;
  public static function go() {
    self::$heh = array();
    self::$heh->bar = 0;  // we should know this can't affect $this->bar
  }
  public function get() {
    var_dump(is_string($this->bar));
    return $this->bar;
  }
}

(new x)->get();
x::go();
