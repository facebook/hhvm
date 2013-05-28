<?php

trait A {
  function test() {
    static $x;
    return ++$x;
  }
  function test8($a, $b, $c, $d, $e, $f, $g, $h) {
    static $x;
    return ++$x;
  }
}
trait X {
  use A;
}
trait Y {
  use A;
}
class T {
  use X, Y {
    X::test insteadof Y;
    X::test as test1;
    Y::test as test2;
    X::test8 insteadof Y;
    X::test8 as test81;
    Y::test8 as test82;
  }
}
var_dump(T::test());
var_dump(T::test1());
var_dump(T::test2());
var_dump(call_user_func("T::test"));
var_dump(call_user_func("T::test1"));
var_dump(call_user_func("T::test2"));
$obj = new T;
var_dump($obj->test());
var_dump($obj->test1());
var_dump($obj->test2());
var_dump(call_user_func(array($obj, "test")));
var_dump(call_user_func(array($obj, "test1")));
var_dump(call_user_func(array($obj, "test2")));
var_dump(T::test8(1, 2, 3, 4, 5, 6, 7, 8));
var_dump(T::test81(1, 2, 3, 4, 5, 6, 7, 8));
var_dump(T::test82(1, 2, 3, 4, 5, 6, 7, 8));
var_dump(call_user_func("T::test8", 1, 2, 3, 4, 5, 6, 7, 8));
var_dump(call_user_func("T::test81", 1, 2, 3, 4, 5, 6, 7, 8));
var_dump(call_user_func("T::test82", 1, 2, 3, 4, 5, 6, 7, 8));
$obj = new T;
var_dump($obj->test8(1, 2, 3, 4, 5, 6, 7, 8));
var_dump($obj->test81(1, 2, 3, 4, 5, 6, 7, 8));
var_dump($obj->test82(1, 2, 3, 4, 5, 6, 7, 8));
var_dump(call_user_func(array($obj, "test8"), 1, 2, 3, 4, 5, 6, 7, 8));
var_dump(call_user_func(array($obj, "test81"), 1, 2, 3, 4, 5, 6, 7, 8));
var_dump(call_user_func(array($obj, "test82"), 1, 2, 3, 4, 5, 6, 7, 8));
