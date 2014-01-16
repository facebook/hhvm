<?php

error_reporting(-1);

//////////////////////////////////////////////////////////////////////

class Foo {
  public function __isset($name) {
    var_dump($name);
    if ($name != 'bar')
      return isset($this->bar);
    else
      return true;
  }
}

function main1() {
  $f = new Foo;
  var_dump(isset($f->foo));
}

//////////////////////////////////////////////////////////////////////

class Foo2 {
  public function __isset($name) {
    var_dump($name);
    return isset($this->foo);
  }
}

function main2() {
  $f = new Foo2;
  var_dump(isset($f->bar));
}

//////////////////////////////////////////////////////////////////////

class CaseFoo {
  public function __isset($name) {
    var_dump($name);
    if ($name === 'bar') return isset($this->Bar);
    if ($name === 'Bar') return isset($this->foo);
    if ($name === 'foo') return isset($this->bar);
    var_dump("shouldn't get here");
  }
}

function main3() {
  $f = new CaseFoo;
  var_dump(isset($f->bar));
}

//////////////////////////////////////////////////////////////////////

main1();
echo "--\n";
main2();
echo "--\n";
main3();
echo "--\n";
