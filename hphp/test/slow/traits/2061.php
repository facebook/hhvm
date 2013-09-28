<?php

trait T {
  private $x = 'init from T';
  function useT() {
 $this->x = 'set from trait';
 }
 }
class A {
  function useA() {
 $this->x = 'set from A';
 }
}
class B extends A {
  use T;
  function useB() {
 $this->x = 'set from B';
 }
}
class C extends B {
  function useC() {
 $this->x = 'set from C';
 }
}
class D extends C {
  function useD() {
 $this->x = 'set from D';
 }
}
$x = new D();
 echo serialize($x), "\n";
$x = new D();
 $x->useT();
 echo serialize($x), "\n";
$x = new D();
 $x->useA();
 echo serialize($x), "\n";
$x = new D();
 $x->useB();
 echo serialize($x), "\n";
$x = new D();
 $x->useC();
 echo serialize($x), "\n";
$x = new D();
 $x->useD();
 echo serialize($x), "\n";
