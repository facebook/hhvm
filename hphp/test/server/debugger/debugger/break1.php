<?php

// Warning: line numbers are sensitive, do not change

function foo($x) {
  $y = $x.'_suffix';
  error_log($y);
}

class cls {
  public function pubObj($x) {
    error_log("pubObj:".$x);
  }
  public static function pubCls($x) {
    error_log("pubCls:".$x);
  }
  public function pubHardBreak($x) {
    error_log("pubHardBreak:".$x);
    hphpd_break();
    error_log("pubHardBreak:done");
  }
}

class derived extends cls {
  public function callPubObj($x) {
    $this->pubObj($x);
  }
  public function callCallPubObj($x) {
    $this->callPubObj($x);
  }
}

error_log('break1.php loaded');
