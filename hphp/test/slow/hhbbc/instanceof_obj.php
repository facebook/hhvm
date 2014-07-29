<?php

class Foo {}
class Bar extends Foo {}

interface IThing {}
class Z implements IThing {}
function get_thing(IThing $x) { return $x; }
function interface_type() { return get_thing(new Z); }

function foo(Foo $x) {
  $y = new Bar;
  var_dump($x instanceof $y);
  var_dump($y instanceof $y);
  $z = interface_type();
  var_dump($y instanceof $z);
}

foo(new Foo);
foo(new Bar);
