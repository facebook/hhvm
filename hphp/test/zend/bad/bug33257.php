<?php
class X {
  protected static $arr = array("a", "b", "c");
  public static function getArr() {
    return self::$arr;
  }
}

//$arr1 = X::getArr();
array_splice(X::getArr(), 1, 1);
print_r(X::getArr());
?>