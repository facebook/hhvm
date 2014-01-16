<?php

error_reporting(-1);

//////////////////////////////////////////////////////////////////////

class Foo {
  public function __set($k, $v) {
    var_dump($k, $v);
    if ($k != 'bar') {
      $this->bar = ($v + 1);
    }
  }
}

function main1() {
  $f = new Foo;
  $f->foo = 2;
}

//////////////////////////////////////////////////////////////////////

class Foo2 {
  public function __set($k, $v) {
    var_dump($k, $v);
    $this->foo = ($v + 1);
  }
}

function main2() {
  $f = new Foo2;
  $f->bar = 2;
}

//////////////////////////////////////////////////////////////////////

class CaseFoo {
  public function __set($k, $v) {
    var_dump($k, $v);
    if ($k === 'bar') { $this->Bar = ($v + 1); return; }
    if ($k === 'Bar') { $this->foo = ($v + 1); return; }
    if ($k === 'foo') { $this->bar = ($v + 1); return; }
    var_dump("shouldn't get here");
  }
}

function main3() {
  $f = new CaseFoo;
  $f->bar = 2;
}

//////////////////////////////////////////////////////////////////////

main1();
echo "--\n";
main2();
echo "--\n";
main3();
echo "--\n";
