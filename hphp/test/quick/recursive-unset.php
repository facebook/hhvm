<?php

error_reporting(-1);

//////////////////////////////////////////////////////////////////////

class Foo {
  public function __unset($name) {
    var_dump($name);
    if ($name != 'bar') unset($this->bar);
  }
}

function main1() {
  $f = new Foo;
  unset($f->foo);
}

//////////////////////////////////////////////////////////////////////

class Foo2 {
  public function __unset($name) {
    var_dump($name);
    unset($this->foo);
  }
}

function main2() {
  $f = new Foo2;
  unset($f->bar);
}

//////////////////////////////////////////////////////////////////////

class CaseFoo {
  public function __unset($name) {
    var_dump($name);
    if ($name === 'bar') { unset($this->Bar); return; }
    if ($name === 'Bar') { unset($this->foo); return; }
    if ($name === 'foo') { unset($this->bar); return; }
    var_dump("shouldn't get here");
  }
}

function main3() {
  $f = new CaseFoo;
  unset($f->bar);
}

//////////////////////////////////////////////////////////////////////

main1();
echo "--\n";
main2();
echo "--\n";
main3();
echo "--\n";
