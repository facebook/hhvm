<?php

class a {
  public static function x() {
    echo 'x';
  }
}
if (0) {
  class b {
  }
}
class b extends a{
  public static function z() {
    self::x();
  }
}
b::x();
