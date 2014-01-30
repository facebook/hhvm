<?php

class y {}
class x {
  private static $set = 12;
  private static $hm = "asd";
  private static $k;
  public static function go() {
    self::$k = new y;
    for ($i = 0; $i < 10; ++$i) {
      self::$set = $i; // enough to fool hphpc into leaving the
                       // IssetS,EmptyS opcodes
      var_dump(empty(self::$set));
      var_dump(isset(self::$set));
    }
  }
}

x::go();
