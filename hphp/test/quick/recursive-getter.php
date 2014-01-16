<?php

error_reporting(-1);

//////////////////////////////////////////////////////////////////////

class Foo {
  public function __get($name) {
    var_dump($name);
    if ($name != 'bar')
      return $this->bar;
    else
      return null;
  }
}

function main1() {
  $f = new Foo;
  $f->foo;
}

//////////////////////////////////////////////////////////////////////

class Foo2 {
  public function __get($name) {
    var_dump($name);
    return $this->foo;
  }
}

function main2() {
  $f = new Foo2;
  var_dump($f->bar);
}

//////////////////////////////////////////////////////////////////////

class CaseFoo {
  public function __get($name) {
    var_dump($name);
    if ($name === 'bar') return $this->Bar;
    if ($name === 'Bar') return $this->foo;
    if ($name === 'foo') return $this->bar;
    var_dump("shouldn't get here");
  }
}

function main3() {
  $f = new CaseFoo;
  var_dump($f->bar);
}

//////////////////////////////////////////////////////////////////////

main1();
echo "--\n";
main2();
echo "--\n";
main3();
echo "--\n";
