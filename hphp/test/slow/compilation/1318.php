<?php

if (isset($g)) {
  class X {
    private static $i = null;
    function foo() {
      self::$i = $this;
    }
  }
}
 else {
  class X {
}
}
