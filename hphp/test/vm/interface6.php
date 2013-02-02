<?php
abstract class B {
  abstract public function foo();
}
abstract class C extends B{}
abstract class D extends C{}
class E extends D {}
