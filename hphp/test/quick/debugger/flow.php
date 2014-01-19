<?php

// Warning: line numbers are sensitive, do not change

function foo($a, $b) {
  $c = $b - $a;
  error_log("foo:".$c);
}

class cls {
  public function pub($x) {
    error_log("in pub:".$x);
    $this->pri($x);
    error_log("out pub:".$x);
  }
  private function pri($x) {
    error_log("in pri:".$x);
    $y = $x + 3; $z = $x * 5;
    foo($y, $z);
    if ($x == 3) {
      hphpd_break();
    }
    if ($x == 4) {
      bigLoop(100000); // Slow enough that incorrect stepping will time out.
    }
    if ($x == 5 || $x == 6) {
      baz($x);
    }
    error_log("out pri:".$x);
  }
}

function bigLoopAdder($a, $b) {
  return $a + $b;
}

function bigLoop($a) {
  $b = 0;
  while (--$a) {
    $b += bigLoopAdder($a, $b);
  }
  return $b;
}

function thrower($a) {
  throw new Exception();
}

function baz($a) {
  try {
    $a = thrower($a);
  } catch (Exception $e) {
    $a = 0;
  }

  return $a;
}

function test($x) {
  error_log("test ".$x);
  $obj = new cls();
  $obj->pub($x);
  error_log("test done ".$x);
}

error_log('flow_t.php loaded');
