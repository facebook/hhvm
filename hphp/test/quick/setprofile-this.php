<?php
// Copyright 2004-present Facebook. All Rights Reserved.

trait logger {
  public function __construct() {
    echo "\n".__CLASS__." constructing\n";
  }
  public function __destruct() {
    echo "\n".__CLASS__." destructing\n";
  }
}

class A {
  use logger;
}
class C {
  use logger;
  public function method() {
    echo "\nC method\n";
    return new A;
  }
}

function profiler($event, $name, $info) {
  static $indent = 2;
  if ($name == 'get_class') return;

  if ($event == 'exit') --$indent;
  printf("\n%s%s %s: %s\n", str_repeat('  ', $indent), $event,
         $name, serialize($info));
  if ($event == 'enter') ++$indent;

  static $threw = false;
  if ($event == 'exit' &&
      ((!$threw && strncmp('C::', $name, 3) == 0) ||
       $name === 'C::method')) {
    $threw = true;
    throw new Exception($name);
  }
}

fb_setprofile('profiler');

function main() {
  try {
    new C();
  } catch (Exception $e) {
    echo "\nCaught ".$e->getMessage()."\n";
  }

  try {
    (new C())->method();
  } catch (Exception $e) {
    echo "\nCaught ".$e->getMessage()."\n";
  }
}
main();
