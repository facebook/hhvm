<?php

// Warning: line numbers are sensitive, do not change

function bar($x, $y) {
  $obj = new cls();
  error_log("bar:".$x." ".$y);
  if ($y <= 0) {
    return $x;
  }
  $obj->pub($x, $y - 1);
}

function foo($x, $y) {
  error_log("foo:".$x." ".$y);
  return bar($x, $y);
}

class cls {
  public function pub($x, $y) {
    error_log("in pub:".$x." ".$y);
    $v = $this->pri($x, $y);
    error_log("out pub:".$x." ".$y);
    return $v;
  }
  private function pri($x, $y) {
    error_log("in pri:".$x." ".$y);
    foo($x, $y);
    if ($x == 3) {
      hphpd_break();
    }
    error_log("out pri:".$x." ".$y);
  }
}

function test($x, $y) {
  error_log("test ".$x." ".$y);
  $obj = new cls();
  $obj->pub($x, $y);
  error_log("test done ".$x." ".$y);
}

error_log('stack1.php loaded');
