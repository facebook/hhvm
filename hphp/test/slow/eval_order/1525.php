<?php

function f($x) {

  var_dump($x, EvalOrder1525Php::$a);
  return $x;
}
class X implements ArrayAccess {
  function OffsetGet($n) {
    echo 'get:';
    var_dump($n);
    return (string)(int)$n == (string)$n ? $this : $n;
  }
  function OffsetSet($n, $v) {
    $this->{$n} = $v;
    echo 'set:';
    var_dump($n, $v);
  }
  function OffsetExists($n) {
    return $n == 'foo';
  }
  function OffsetUnset($n) {
  }
  function __get($n) {
    return $this->OffsetGet($n);
  }
  function __set($n, $v) {
    return $this->OffsetSet($n, $v);
  }
}
EvalOrder1525Php::$a = new X;
<<__EntryPoint>>
function test() {
  EvalOrder1525Php::$a[f(0)]->{
    f(1)
  }[f(2)] = EvalOrder1525Php::$a[f(3)][f(4)][f(5)]->foo;
  var_dump(EvalOrder1525Php::$a[f(6)]['fuz'].f(7));
  list(
    EvalOrder1525Php::$a[f(10)][f(11)][f(12)],
    EvalOrder1525Php::$a[f(20)][f(21)][f(22)],
  ) = EvalOrder1525Php::$a;
  EvalOrder1525Php::$a->{f(0)}[f(1)]->{f(2)} = EvalOrder1525Php::$a->{f(3)}
    ->{f(4)}
    ->bar;
}

abstract final class EvalOrder1525Php {
  public static $a;
}
