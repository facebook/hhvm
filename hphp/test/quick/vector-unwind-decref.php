<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class logger2 {
  public function __construct() {
    echo "logger2 constructing\n";
  }

  public function __destruct() {
    echo "logger2 destructing\n";
  }

  public function __set($name, $value) {
    echo "set was called, throwing an exception\n";
    throw new Exception('nope');
  }
}

class logger {
  public function __construct() {
    echo "logger constructing\n";
  }

  public function __destruct() {
    echo "logger destructing\n";
  }

  public function __set($name, $value) {
    echo "set was called\n";
  }

  public function __get($name) {
    echo "get was called\n";
    return 10;
  }
}

class c {
  public function __construct() {
    echo "c constructing\n";
  }

  public function __destruct() {
    echo "c destructing\n";
  }

  public function __get($name) {
    echo "returning new logger\n";
    return new logger;
  }
}

class d {
  public function __construct() {
    echo "d constructing\n";
  }

  public function __destruct() {
    echo "d destructing\n";
  }

  public function __get($name) {
    echo "returning new logger2\n";
    return new logger2;
  }
}

function main() {
  echo "in main\n";
  echo "creating new c\n";
  $o = new c();
  echo "calling c.__get() and logger.__set()\n";
  $o->prop->blah = 'something';


  echo "calling c.__get() and logger.__get()\n";
  $x = $o->porp->halb;
  echo "got value " . $x . "\n";

  echo "creating new d\n";
  $b = new d();
  echo "calling d.__get() and logger2.__set()\n";
  $b->fake->anotherfake = 'ello';
  # exception!
}

try {
  echo "calling main\n";
  main();
} catch (Exception $e) {
  echo "Caught exception\n";
}

echo "last line\n";
