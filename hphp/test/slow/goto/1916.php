<?php

class MyException extends Exception {
}
class MyOtherException extends Exception {
}
function baz($x) {
  var_dump('baz: ' . $x);
  if (($x & 7) == 5) throw new Exception('regular');
  if (($x & 7) == 6) throw new MyException('mine');
  if (($x & 7) == 7) throw new MyOtherException('other');
}
function foo($t) {
  $e = $m = $q = new Exception('none');
  if ($t & 8) {
    switch ($t & 3) {
      case 0: goto l0;
      case 1: goto l1;
      case 2: goto l2;
      case 3: goto l3;
    }
  }
  try {
    var_dump('begin try1');
    l0: var_dump('l0');
    try {
      var_dump('begin try2');
      l1: var_dump('l1');
      baz($t);
      var_dump('after baz');
    }
 catch (MyOtherException $q) {
      var_dump($q->getMessage());
    }
    var_dump('after try2');
  }
 catch (Exception $e) {
    l2: var_dump($e->getMessage());
  }
 catch (MyException $m) {
    l3: var_dump($m->getMessage());
  }
  var_dump('after try1');
}
for ($i = 0;
 $i < 16;
 $i++) foo($i);
