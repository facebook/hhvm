<?php

// Warning: line numbers are sensitive, do not change

function foo2($x) {
  $y = $x.'_suffix';
  error_log($y);
}

class cls2 {
  public function pubObj($x) {
    error_log("pubObj2:".$x);
  }
  public static function pubCls($x) {
    error_log("pubCls2:".$x);
  }
}

error_log('break2.php loaded');
