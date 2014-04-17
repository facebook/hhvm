<?php

trait T {
  protected $foo;
}

class X {
  use T;
}

class Y extends X {
  static $foo;
}

echo "Ok\n";
