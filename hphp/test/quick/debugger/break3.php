<?php
namespace TestNs;

// Warning: line numbers are sensitive, do not change

function foo($x) {
  $y = $x.'_suffix';
  \error_log($y);
}

class cls {
  public function pubObj($x) {
    \error_log("pubObj:".$x);
  }
  public static function pubCls($x) {
    \error_log("pubCls:".$x);
  }
}

\error_log('break3.php loaded');
