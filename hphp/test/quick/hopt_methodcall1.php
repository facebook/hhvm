<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.


echo "Starting\n";

// tests FPushFuncD
function hello() {
  echo "hello";
  return 1;
}

// tests FPushFunc
function doCall($f) {
  echo "doCall(";
  if ($f()) {
    echo ") -- doCall got true\n";
  } else {
    echo ") -- doCall got false\n";
  }
}

class C {
  public function method1() {
    $x = 1;
    echo "method1\n";
    if ($x) {
      doCall("hello");
      return hello();
    }
    return 0;
  }
  public function doPr($x) {
    echo $x;
    echo "\n";
    if ($x) {
      return 1;
    }
    return 2;
  }
  public function foo() {
    echo "foo\n";
    if ($this->method1()) {
      if ($this->doPr(true)) {
        if ($this->doPr(49)) {
          return 1;
        }
      }
      return 1;
    }
    return 2;
  }
}

$c = new C;
$val = $c->foo();
echo $val;
echo "\n";
echo "Done\n";
