<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

class C {
  function f1($a, $b) {
    return $a + $b;
  }

  function f2($a) {
    return $a + 1;
  }

  // Note: we have a bug in hackir if we remove the $this's below
  function test($a) {
    $x = $this->f1($this->f2($a), $a);
    echo $x;
    echo "\n";
  }
}

//$x = new C();
//$x->test(1);

function f3($a, $b) {
  return $a + $b;
}

function f4($a) {
  return $a + 1;
}

/* Note: we have a bug in hackir if we remove the $this's below */
function test($a, $f1, $f2) {
  $x = $f1($f2($a), $a);
  echo $x;
  echo "\n";
}

test(1, "f3", "f4");
