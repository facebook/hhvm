<?php
error_reporting(E_ALL);

function __autoload($s) { echo "[load $s]"; }
interface I {}
class A { }
class B extends A { }
class C implements I { }
class D extends C { }
class E extends B implements I { }

class UnexpectedSerializedClass extends Exception {}
function main() {
  $v = serialize(array(new A, new B, new C, new D, new E));
  $run = $opts ==> {
    try {
      printf("%s (%s)\n",
             join("", array_map(
                    $x ==> get_class($x)[0],
                    unserialize($v, $opts))),
             json_encode($opts));
    } catch (Exception $e) {
      printf("%s %s %s\n", get_class($e), $e->getMessage(), json_encode($opts));
    }
  };
  foreach (vec[false, true] as $subclasses) {
    $check = $xs ==> $run(shape('include_subclasses' => $subclasses,
                                'allowed_classes' => $xs));
    $check(array());
    $check(array('A'));
    $check(array('B'));
    $check(array('C'));
    $check(array('D'));
    $check(array('E'));
    $check(array('I'));
    $check(array('A', 'I'));
    $check(vec['A', 'I']);
    $run(shape('include_subclasses' => $subclasses,
               'allowed_classes' => vec['A','B','C','D','E','I'],
               'throw' => 'UnexpectedSerializedClass'));
    $run(shape('include_subclasses' => $subclasses,
               'allowed_classes' => vec['A','I'],
               'throw' => 'UnexpectedSerializedClass'));
    $run(shape('include_subclasses' => $subclasses,
               'allowed_classes' => false,
               'throw' => false));
    $run(shape('include_subclasses' => $subclasses,
               'allowed_classes' => false,
               'throw' => 'UnexpectedSerializedClass'));
  }
  unserialize('O:3:"Foo":0:{}',
    shape('allowed_classes' => vec['I']));
  unserialize('O:3:"Bar":0:{}',
    shape('allowed_classes' => vec['Feh'],
          'include_subclasses' => true));
  unserialize('O:3:"Baz":0:{}',
    shape('allowed_classes' => vec['Fiddle'],
          'include_subclasses' => true,
          'throw' => 'MadeUp'));
}

main();
