<?php
namespace TestNs;

// Warning: line numbers are sensitive, do not change

function foo($x) {
  $y = $x.'_TestNs';
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

namespace TestNs\Nested;

function foo($x) {
  $y = $x.'_TestNs\Nested';
  \error_log($y);
}

class cls {
  public function pubObj($x) {
    \error_log("pubObj:".$x.'_TestNs\Nested');
  }
  public static function pubCls($x) {
    \error_log("pubCls:".$x.'_TestNs\Nested');
  }
}

\error_log('break3.php loaded');
