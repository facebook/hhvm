<?php
error_reporting(E_ALL);

interface I {}
class A { }
class B extends A { }
class C implements I { }
class D extends C { }
class E extends B implements I { }

function main() {
  $v = serialize(array(new A, new B, new C, new D, new E));
  $check = $wl ==> {
    printf("%s (%s)\n",
           join("", array_map(
                  $x ==> get_class($x)[0],
                  unserialize($v, array('include_subclasses' => true,
                                        'allowed_classes' => $wl)))),
           join(",", $wl));
    "\n";
  };
  $check(array());
  $check(array('A'));
  $check(array('B'));
  $check(array('C'));
  $check(array('D'));
  $check(array('E'));
  $check(array('I'));
  $check(array('A', 'I'));
}

main();
