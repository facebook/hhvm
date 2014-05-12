<?php

error_reporting(E_NOTICE);

class D {
  public $id;

  public function __construct($id) {
    $this->id = $id;
  }

  public function __destruct() {
    echo "Instance $this->id destroyed!\n";
  }
}

function main() {
  $i1 = new D('+=');
  $i2 = new D('-=');
  $i3 = new D('*=');
  $i4 = new D('/=');
  $i5 = new D('%=');
  $i6 = new D('**=');
  $i7 = new D('&=');
  $i8 = new D('|=');
  $i9 = new D('^=');

  $i1 += 1;
  $i2 -= 1;
  $i3 *= 1;
  $i4 /= 1;
  $i5 %= 1;
  $i6 **= 1;
  $i7 &= 1;
  $i8 |= 1;
  $i9 ^= 1;

  var_dump($i1);
  var_dump($i2);
  var_dump($i3);
  var_dump($i4);
  var_dump($i5);
  var_dump($i6);
  var_dump($i7);
  var_dump($i8);
  var_dump($i9);
}

main();
