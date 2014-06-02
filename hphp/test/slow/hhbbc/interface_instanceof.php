<?php

abstract class Base {}
abstract class Derived extends Base {}

interface Heh {}
interface Heh2 {}

class D1 extends Base implements Heh, Heh2 {}
class D2 extends Derived implements Heh2 {}

class Factory {
  static function create($t) {
    if (!$t) return new D1;
    return new D2;
  }
}

function main($k) {
  $x = Factory::create($k);
  if ($x instanceof Derived) {
    echo "derived\n";
  }
}

main(2);
