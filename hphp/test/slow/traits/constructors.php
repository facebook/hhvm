<?php

trait T {
  function x() { echo __METHOD__, "\n"; }
  function y() { echo __METHOD__, "\n"; }
  function z() { echo __METHOD__, "\n"; }
}
trait U {
    function __construct() { echo __METHOD__, "\n"; }
}
class B { use T; }
class X extends B {}
(new X)->x();
class Y extends B {
  function __construct() { echo __METHOD__, "\n"; }
}
(new Y)->y();
class Z extends B {
    use U;
}
(new Z)->z();
